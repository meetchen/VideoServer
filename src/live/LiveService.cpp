#include "live/LiveService.h"
#include "base/StringUtils.h"
#include "base/Config.h"
#include "live/base/LiveLog.h"
#include "base/Task.h"
#include "base/TTime.h"
#include "mmedia/rtmp/RtmpServer.h"


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