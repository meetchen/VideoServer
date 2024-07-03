/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 19:45:45
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-03 15:56:48
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpServer.cpp
 * @Description: IMP RTMPSERVER.H
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/RtmpServer.h"
#include "mmedia/base/MMediaLog.h"
#include <functional>
#include <iostream>


using namespace vdse::mmedia;
using namespace std;


RtmpServer::RtmpServer(EventLoop *loop, InetAddress &local, RtmpHandler *handler)
:TcpServer(loop, local), rtmp_handler_(handler)
{

}

RtmpServer::~RtmpServer()
{
    Stop();
}

void RtmpServer::Start() 
{
    TcpServer::SetNewConnectionCallBack(std::bind(&RtmpServer::OnNewConneciton, this, std::placeholders::_1));
    TcpServer::SetDestroyConnectionCallBack(std::bind(&RtmpServer::OnDestroyed, this, std::placeholders::_1));
    TcpServer::SetMsgCallBack(std::bind(&RtmpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    TcpServer::SetWriteCompleteCallBack(std::bind(&RtmpServer::OnWriteComplete, this, std::placeholders::_1));
    TcpServer::SetActiveCallBack(std::bind(&RtmpServer::OnActive, this, std::placeholders::_1));
    TcpServer::Start();
}

void RtmpServer::Stop() 
{
    TcpServer::Stop();
}

void RtmpServer::OnNewConneciton(const TcpConnectionPtr &conn)
{
    if (rtmp_handler_)
    {
       rtmp_handler_->OnNewConnection(conn);
    }
    auto shake = std::make_shared<RtmpContext>(conn, nullptr);
    conn -> SetContext(kRtmpContext, shake);
    shake -> StartHandshake();
}

void RtmpServer::OnDestroyed(const TcpConnectionPtr &conn)
{
    if (rtmp_handler_)
    {
        rtmp_handler_->OnConnectionDestroy(conn);
    }
    conn->ClearContext(kRtmpContext);

}

void RtmpServer::OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf)
{
    auto shake = conn -> GetContext<RtmpContext>(kRtmpContext);
    if (shake)
    {
        int ret = shake -> Parse(buf);
        // 握手成功
        if (ret == 0)
        {
            RTMP_TRACE << " host " << conn -> PeerAddr().ToIpPort() << " handshake finish \n";
        }
        else if (ret == -1)
        {
            conn -> ForceClose();
            RTMP_ERROR << " host " << conn -> PeerAddr().ToIpPort() << "handshake error ";
            return;
        }
    }
}

void RtmpServer::OnWriteComplete(const ConnectionPtr &conn)
{
    auto shake = conn -> GetContext<RtmpContext>(kRtmpContext);
    if (shake)
    {
        shake -> OnwriteComplete();
    }
}

void RtmpServer::OnActive(const ConnectionPtr &conn)
{
    if (rtmp_handler_)
    {
        rtmp_handler_ -> OnActive(conn);
    }
}
