/**
 * @FilePath     : /VideoServer/src/network/UdpServer.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 17:43:53
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/UdpServer.h"

using namespace vdse::network;

UdpServer::UdpServer(EventLoop *loop, const InetAddress &server)
:UdpSocket(loop, -1, InetAddress(), server), server_(server)
{

}
UdpServer::~UdpServer()
{
    Stop();
}

void UdpServer::Start()
{
    loop_->RunInLoop([this](){
        Open();
    });
}
void UdpServer::Stop()
{
    loop_->RunInLoop([this](){
       loop_->DelEvent(std::dynamic_pointer_cast<UdpServer>(shared_from_this()));
       OnClose();
    });
}

void UdpServer::Open()
{
    loop_->AssertLoopInThread();
    fd_ = SocketOpt::CreatNonblockingUdpSocket(AF_INET);
    if (fd_ < 0)
    {
        OnClose();
        return;
    }
    loop_->AddEvent(std::dynamic_pointer_cast<UdpServer>(shared_from_this()));
    SocketOpt opt(fd_);
    opt.BindAddress(server_);
}