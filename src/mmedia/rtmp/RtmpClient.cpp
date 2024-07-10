/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpClient.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-07 15:56:32
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "mmedia/rtmp/RtmpClient.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/rtmp/RtmpContext.h"

using namespace vdse::mmedia;

RtmpClient::RtmpClient(EventLoop *loop, RtmpCallBack* rtmp_callback)
:loop_(loop), rtmp_callback_(rtmp_callback)
{

}
  
RtmpClient::~RtmpClient()
{
    
}


void RtmpClient::SetCloseCallback(const CloseConnectionCallBack &cb)
{
    close_cb_ = cb;
}

void RtmpClient::SetCloseCallback(CloseConnectionCallBack &&cb)
{
    close_cb_ = std::move(cb);
}

void RtmpClient::Play(const std::string &url)
{
    is_player_ = true;
    url_ = url;
    CreateTcpClient();
}
void RtmpClient::Publish(const std::string &url)
{   
    is_player_ = false;
    url_ = url;
    CreateTcpClient();
}
void RtmpClient::Send(PacketPtr &&data)
{

}
void RtmpClient::OnWriteComplete(const TcpConnectionPtr &conn)
{
    RTMP_TRACE << "RtmpClient::OnWriteComplete(const TcpConnectionPtr &conn) \n";
    auto context = conn -> GetContext<RtmpContext>(kRtmpContext);
    if (context)
    {
        RTMP_TRACE << "RtmpClient::OnWriteComplete(const TcpConnectionPtr &conn) \n";

        context -> OnWriteComplete();
    }
}
void RtmpClient::OnConnection(const TcpConnectionPtr& conn,bool connected)
{   
    if (connected)
    {
        auto context = std::make_shared<RtmpContext>(conn, rtmp_callback_, true);
        if (is_player_)
        {
            context -> Play(url_);
        }
        else
        {
            context -> Publish(url_);
        }
        conn->SetContext(kRtmpContext, context);
        context -> StartHandshake();
    }

}
void RtmpClient::OnMessage(const TcpConnectionPtr& conn,MsgBuffer &buf)
{
    auto context = conn -> GetContext<RtmpContext>(kRtmpContext);
    if (context)
    {
        auto ret = context -> Parse(buf);
        if (ret == -1)
        {
            RTMP_ERROR << "message parse error";
            conn -> ForceClose();
        }
    }
}   
bool RtmpClient::ParseUrl(const std::string &url)
{
    // 如果没有 rtmp://
    if (url.size() > 7)
    {
        uint16_t port = 1935;

        // 找去掉协议头后面的 第一个 ： or /
        auto pos = url.find_first_of(":/", 7);
        if (pos != std::string::npos)
        {   
            // 获取域名
            std::string domain = url.substr(7, pos - 7);

            if (url.at(pos) == ':')
            {
                auto pos_next = url.find_first_of("/", pos + 1);
                if (pos_next != std::string::npos)
                {
                    port = std::atoi(url.substr(pos + 1, pos_next - pos).c_str());
                }
            }
            // TODO 考虑使用DNS解析域名
            addr_.SetAddr(domain);
            addr_.SetPort(port);
            return true;
        }
    }
    return false;
}

void RtmpClient::CreateTcpClient()
{
    auto ret = ParseUrl(url_);
    if(!ret)
    {
        RTMP_ERROR << "invalid url:" << url_;
        if(close_cb_)
        {
            close_cb_(nullptr);
        }
        return;
    }
    tcp_client_ = std::make_shared<TcpClient>(loop_,addr_);
    tcp_client_->SetWriteCompleteCallBack(std::bind(&RtmpClient::OnWriteComplete,this,std::placeholders::_1));
    tcp_client_->SetRecvMsgCallBack(std::bind(&RtmpClient::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
    tcp_client_->SetCloseCallBack(close_cb_);
    tcp_client_->SetConnectCallBack(std::bind(&RtmpClient::OnConnection,this,std::placeholders::_1,std::placeholders::_2));
    tcp_client_->Connect();
}