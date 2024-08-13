#include "live/LiveService.h"
#include "base/StringUtils.h"
#include "base/Config.h"
#include "live/base/LiveLog.h"
#include "base/Task.h"
#include "base/TTime.h"
#include "mmedia/rtmp/RtmpServer.h"
#include "mmedia/http/HttpServer.h"
#include "mmedia/http/HttpUtils.h"
#include "mmedia/http/HttpContext.h"
#include "network/DnsService.h"
#include <iostream>


using namespace vdse::live;
using namespace vdse::mmedia;

namespace
{
    static SessionPtr null_sesssion;
}


SessionPtr LiveService::CreateSession(const std::string &session_name)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto it = sessions_.find(session_name);
    if (it != sessions_.end())
    {
        return it -> second;
    }

    auto list = StringUtils::SplitString(session_name, "/");

    if (list.size() != 3)
    {
        LIVE_ERROR << "create session failed. Invalid session name:" << session_name;
        return null_sesssion;
    }

    auto config = sConfigMg->GetConfig();
    auto app_info = config->GetAppInfo(list[0], list[1]);

    if (!app_info)
    {
        LIVE_ERROR << "create session failed. cant found config. domain: " << list[0] << " app:" << list[1];
        return null_sesssion;
    }

    auto session = std::make_shared<Session>(session_name);
    session->SetAppInfo(app_info);
    sessions_.emplace(session_name, session);
    LIVE_DEBUG << "create session success. session_name: " << session_name << " now: " << base::TTime::NowMS();

    return session;
}
SessionPtr LiveService::FindSession(const std::string&  session_name)
{
    std::lock_guard<std::mutex> lk(lock_);

    auto it = sessions_.find(session_name);
    if (it != sessions_.end())
    {
        return it -> second;
    }
    return null_sesssion;
}
bool LiveService::CloseSession(const std::string& session_name)
{
    SessionPtr s;

    // 尽可能缩小竞争区
    {
        std::lock_guard<std::mutex> lk(lock_);
        auto it = sessions_.find(session_name);
        if (it != sessions_.end())
        {
            s = it->second;
            sessions_.erase(it);
        }
    }

    if (s)
    {
        LIVE_INFO << " close session:" << s->SessionName()  << " now:" << base::TTime::NowMS();
        s->Clear();
    }

    return true;
}
void LiveService::OnTimer(const TaskPtr &task)
{
    std::lock_guard<std::mutex> lk(lock_);
    for (auto it = sessions_.begin(); it != sessions_.end();)
    {
        auto &session = it -> second;
        if (session->IsTimeOut())
        {
            LIVE_INFO << "session:" << session->SessionName() 
                    << " is timeout. close it. Now:" << base::TTime::NowMS();
            session->Clear();
            it = sessions_.erase(it);
        }
        else
        {
            ++it;
        }
    }
    task->Restart();
}

void LiveService::OnNewConnection(const TcpConnectionPtr &conn) 
{

}
void LiveService::OnConnectionDestroy(const TcpConnectionPtr &conn) 
{
    auto user = conn->GetContext<User>(kUserContext);
    if (user)
    {
        user->GetSession()->CloseUser(user);
    }
}
void LiveService::OnActive(const ConnectionPtr &conn) 
{
    auto user = conn->GetContext<PlayerUser>(kUserContext);
    if (user && user->GetUserType() >= UserType::kUserTypePlayerPav)
    {
        user->PostFrames();
    }
    else
    {
        LIVE_DEBUG << " no user found, host : " << conn->PeerAddr().ToIpPort();
    }
}

void LiveService::OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data)
{
    // auto user = conn->GetContext<User>(kUserContext);
    // if (user)
    // {
    //     conn->ForceClose();
    // }
    // user->GetStream()->AddPacket(std::move(data));
}
void LiveService::OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data)  
{
    auto user = conn->GetContext<User>(kUserContext);
    if (!user)
    {
        LIVE_ERROR << "no found user. host:" << conn->PeerAddr().ToIpPort();
        conn->ForceClose();
        return;
    }
    user->GetStream()->AddPacket(std::move(data));
}

bool LiveService::OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param)
{
    LIVE_DEBUG << " on play session name:" << session_name 
                << " param : " << param
                << " host: " << conn->PeerAddr().ToIpPort()
                << " time: " << vdse::base::TTime::NowMS();
    // 创建会话                
    auto s = CreateSession(session_name);
    if (!s)
    {
        LIVE_ERROR << "create session failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    }
    // 创建一个播放用户
    auto user = s->CreatePlayer(session_name, conn, UserType::kUserTypePlayerRtmp, param);

    if (!user)
    {
        LIVE_ERROR << "create user failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    }
    // 将用户设置到上下文中去
    conn->SetContext(kUserContext, user);

    s->AddPlayer(std::dynamic_pointer_cast<PlayerUser>(user));
    return true;
}
bool LiveService::OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param)
{
    LIVE_DEBUG << " on publish session name:" << session_name 
                << " param : " << param
                << " host: " << conn->PeerAddr().ToIpPort()
                << " time: " << vdse::base::TTime::NowMS();
    // 创建会话                
    auto s = CreateSession(session_name);
    if (!s)
    {
        LIVE_ERROR << "create session failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    }
    // 创建一个推流用户
    auto user = s->CreatePublisher(session_name, conn, UserType::kUserTypePublishRtmp, param);

    if (!user)
    {
        LIVE_ERROR << "create user failed.session name:" << session_name;
        conn->ForceClose();
        return false;
    }
    // 将用户设置到上下文中去
    conn->SetContext(kUserContext, user);
    s->SetPublisher(user);
    return true;
}

void LiveService::Start()
{
    auto config = sConfigMg->GetConfig();
    // 事件线程池
    pool_ = new EventLoopThreadPool(config->thread_nums_, config->cpu_start_, config->cpus_);
    pool_->Start();

    sDnsService->Start();

    // 获取配置文件中定义的文件
    auto services = config->GetServiceInfos();
    auto loops = pool_->GetLoops();

    for (auto &lp : loops)
    {
        for (auto &service : services)
        {

            if (service->protocol == "RTMP" || service->protocol == "rtmp")
            {
                InetAddress local(service->addr, service->port);
                RtmpServer *server = new RtmpServer(lp, local, this);
                servers_.push_back(server);
                servers_.back()->Start();
            }
            else if (service->protocol == "HTTP" || service->protocol == "http")
            {
                InetAddress local(service->addr, service->port);
                auto server = new HttpServer(lp, local, this);
                servers_.push_back(server);
                servers_.back()->Start();
            }
        }
    }
    TaskPtr task = std::make_shared<Task>(std::bind(&LiveService::OnTimer, this, std::placeholders::_1), 7  * 1000);
    sTaskMg->Add(task);
}

void LiveService::Stop()
{

}

EventLoop* LiveService::GetNextLoop()
{
    return pool_->GetNextLoop();
}

void LiveService::OnSent(const TcpConnectionPtr &conn)
{

}
bool LiveService::OnSentNextChunk(const TcpConnectionPtr &conn)
{
    return false;
}

void LiveService::OnRequest(const TcpConnectionPtr &conn,const HttpRequestPtr &req,const PacketPtr &packet)
{
    auto http_cxt = conn->GetContext<HttpContext>(kHttpContext);
    if(!http_cxt)
    {
        LIVE_ERROR << "no found http context.something must be wrong.";
        return;
    }
    if(req->IsRequest())
    {
        LIVE_DEBUG << "req method:" << req->Method() << " path:" << req->Path();
    }
    else 
    {
        LIVE_DEBUG << "req code:" << req->GetStatusCode() << " msg:" << HttpUtils::ParseStatusMessage(req->GetStatusCode());
    }

    auto headers = req->Headers();
    for(auto const &h:headers)
    {
        LIVE_DEBUG << h.first << ":" << h.second;
    }
    
    if(req->IsRequest())
    {
        //http://ip:port/domain/app/stream.flv
        //http://ip:port/domain/app/stream/filename.flv
        auto list = base::StringUtils::SplitString(req->Path(),"/");
        if(list.size()<4)
        {
            auto res = HttpRequest::NewHttp400Response();
            http_cxt->PostRequest(res);
            return ;
        }
        const std::string &domain = list[1];
        const std::string &app = list[2];
        string filename = list[3];
        std::string stream_name;
        if(list.size()>4)
        {
            filename = list[4];
            stream_name = list[3];
        }
        else 
        {
            stream_name = base::StringUtils::FileName(filename);
        }

        std::string session_name = domain+"/"+app+"/"+stream_name;
        LIVE_DEBUG << "request session name:" << session_name;
        auto s = CreateSession(session_name);
        if(!s)
        {
            LIVE_ERROR << "cant create session  name:" << session_name;
            auto http_cxt = conn->GetContext<HttpContext>(kHttpContext);
            if(http_cxt)
            {
                auto res = HttpRequest::NewHttp404Response();
                http_cxt->PostRequest(res);
                return ;
            }
        }        
        std::string ext = base::StringUtils::Extension(filename);
        // if(ext == "flv")
        // {
        //     auto user = s->CreatePlayerUser(conn,session_name,"",UserType::kUserTypePlayerFlv);
        //     if(!user)   
        //     {
        //         LIVE_ERROR << "cant create user  session name:" << session_name;
        //         auto res = HttpRequest::NewHttp404Response();
        //         http_cxt->PostRequest(res);
        //         return ;  
        //     }    
        //     conn->SetContext(kUserContext,user);
        //     auto flv = std::make_shared<FlvContext>(conn,this);
        //     conn->SetContext(kFlvContext,flv);
        //     s->AddPlayer(std::dynamic_pointer_cast<PlayerUser>(user));     
        // }
        // else if(ext == "m3u8")
        // {
        //     auto playlist = s->GetStream()->PlayList();
        //     if(!playlist.empty())
        //     {
        //         auto res = std::make_shared<HttpRequest>(false);
        //         res->AddHeader("server","tmms");
        //         res->AddHeader("content-length",std::to_string(playlist.size()));
        //         res->AddHeader("content-type","application/vnd.apple.mpegurl");
        //         res->SetStatusCode(200);
        //         res->SetBody(playlist);
        //         LIVE_DEBUG << "http:\n" << res->AppendToBuffer();
        //         http_cxt->PostRequest(res);
        //     }
        //     else 
        //     {
        //         auto res = HttpRequest::NewHttp404Response();
        //         http_cxt->PostRequest(res);
        //         return ;  
        //     }
        // }
        // else if(ext == "ts")
        // {
        //     LIVE_DEBUG << "request ts:" << filename;
        //     auto frag = s->GetStream()->GetFragment(filename);
        //     if(frag)
        //     {
        //         auto res = std::make_shared<HttpRequest>(false);
        //         res->AddHeader("server","tmms");
        //         res->AddHeader("content-length",std::to_string(frag->Size()));
        //         res->AddHeader("content-type","video/MP2T");
        //         res->SetStatusCode(200);
        //         http_cxt->PostRequest(res->MakeHeaders(),frag->FragmentData());
        //     }
        // }
    }
}