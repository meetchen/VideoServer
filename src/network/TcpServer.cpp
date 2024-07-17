/**
 * @FilePath     : /VideoServer/src/network/TcpServer.cpp
 * @Description  :  imp TcpServer
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-29 21:46:54
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/TcpServer.h"
#include "network/base/Network.h"
#include <iostream>

using namespace vdse::network;


TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr)
:loop_(loop),addr_(addr)
{
    acceptor_ = std::make_shared<Acceptor>(loop, addr);
}

TcpServer::~TcpServer()
{
    
}

void TcpServer::SetNewConnectionCallBack(const NewConnctionCallBack & cb)
{
    new_connection_cb_ = cb;
}

void TcpServer::SetNewConnecitonCallBack(NewConnctionCallBack &&cb)
{
    new_connection_cb_ = std::move(cb);
}


void TcpServer::SetDestroyConnectionCallBack(const DestroyConnectionCallBack & cb)
{
    destroy_connection_cb_ = cb;
}

void TcpServer::SetDestroyConnectionCallBack(DestroyConnectionCallBack &&cb)
{
    destroy_connection_cb_ = std::move(cb);
}


void TcpServer::SetActiveCallBack(const ActiveCallBack &cb)
{
    active_cb_ = cb;   
}
void TcpServer::SetActiveCallBack(ActiveCallBack &&cb)
{
    active_cb_ = std::move(cb);
}

void TcpServer::SetMsgCallBack(const MsgCallBack &cb)
{
    message_cb_ = cb;
}
void TcpServer::SetMsgCallBack(MsgCallBack &&cb)
{
    message_cb_ = std::move(cb);
}


void TcpServer::SetWriteCompleteCallBack(const WriteCompleteCallBack &cb)
{
    write_complete_cb_ = cb;
}

void TcpServer::SetWriteCompleteCallBack(WriteCompleteCallBack &&cb)
{
    write_complete_cb_ = std::move(cb); 
}

void TcpServer::OnAccept(int fd, const InetAddress &addr)
{
    NETWORK_TRACE << "new connection fd: " << fd << " host : " << addr.ToIpPort() ;
    // 创建一个tcp链接，在同一个事件循环里面处理
    auto conn = std::make_shared<TcpConnection>(loop_, fd, addr_, addr);

    // 设置关闭链接的回调，使用bind进行绑定
    conn ->SetCloseCallBack(std::bind(&TcpServer::OnConnectionClose,this, std::placeholders::_1));

    if (write_complete_cb_)
    {
        conn->SetWriteCompleteCallBack(write_complete_cb_);
    }

    if (active_cb_)
    {
        conn->SetActiveCallBack(active_cb_);
    }


    conn->SetRecvMsgCallBack(message_cb_);

    connections_.emplace(conn);

    conn->EnableCheckIdleTimeOut(30);

    if (new_connection_cb_)
    {
        new_connection_cb_(conn);
    }

    loop_->AddEvent(conn);
}


void TcpServer::Start()
{
    acceptor_->SetAcceptCallBack(std::bind(&TcpServer::OnAccept, this, std::placeholders::_1, std::placeholders::_2));
    acceptor_->Start();
}

void TcpServer::Stop()
{
    acceptor_->Stop();
}


void TcpServer::OnConnectionClose(const TcpConnectionPtr &conn)
{
    NETWORK_TRACE << "host : " << conn->PeerAddr().ToIpPort()  << " cloesd ";
    loop_ -> AssertLoopInThread();

    connections_.erase(conn);
    loop_ ->DelEvent(conn);
    if (destroy_connection_cb_)
    {
        destroy_connection_cb_(conn);
    }
}
