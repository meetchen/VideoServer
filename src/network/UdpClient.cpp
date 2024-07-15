/**
 * @FilePath     : /VideoServer/src/network/UdpClient.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 15:47:27
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#include "network/UdpClient.h"
#include "network/base/SocketOpt.h"
#include "network/base/Network.h"

using namespace vdse::network;


UdpClient::UdpClient(EventLoop *loop, const InetAddress &server)
:UdpSocket(loop, -1, InetAddress(), server), server_addr_(server)
{
    server_addr_.GetSockAddr((struct sockaddr *)&sock_addr_);
}

UdpClient::~UdpClient()
{

}

void UdpClient::Connect()
{
    loop_->RunInLoop([this](){
        ConnectInLoop();
    });
}
void UdpClient::SetConnectedCallBack(const ConnectedCallBack &cb)
{
    connectioned_cb_ = cb;
}

void UdpClient::SetConnectedCallBack(ConnectedCallBack &&cb)
{
    connectioned_cb_ = std::move(cb);
}

void UdpClient::Send(std::list<UdpBufferNodePtr> &list)
{

}
void UdpClient::Send(const char *buf, size_t size)
{
    UdpSocket::Send(buf, size, (struct sockaddr *)&sock_addr_, sock_len_);
}
void UdpClient::OnClose() 
{
    if (connected_)
    {
        loop_->DelEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        connected_ = false;
        UdpSocket::OnClose();
    }
}


void UdpClient::ConnectInLoop()
{
    loop_ -> AssertLoopInThread();
    fd_ = SocketOpt::CreatNonblockingUdpSocket(AF_INET);
    if (fd_ < 0)
    {
        OnClose();
        return;
    }
    connected_ = true;
    loop_->AddEvent(std::dynamic_pointer_cast<UdpClient>(shared_from_this()));
    SocketOpt opt(fd_);
    opt.Connect(server_addr_);
    if (connectioned_cb_)
    {
        connectioned_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()), true);
    }
}