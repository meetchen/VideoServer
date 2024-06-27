#include "network/net/Acceptor.h"
#include <unistd.h>
#include "network/base/Network.h"

using namespace vdse::network;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &addr)
:Event(loop),
addr_(addr)
{

}

Acceptor::~Acceptor()
{
    if (socket_opt_)
    {
        delete socket_opt_;
        socket_opt_ = nullptr;
    }
}


void Acceptor::SetAcceptCallBack(const AcceptCallBack &cb)
{
    accept_cb_ = cb;
}
void Acceptor::SetAcceptCallBack(AcceptCallBack &&cb)
{   
    accept_cb_ = std::move(cb);
}

void Acceptor::Start()
{
    loop_->RunInLoop([this](){
        Open();
    });
}

void Acceptor::Open()
{
    if (fd_ > 0)
    {
        ::close(fd_);
        fd_ = 0;
    }
    
    fd_ = SocketOpt::CreatNonblockingTcpSocket(addr_.IsIpV6()? AF_INET6 : AF_INET);
    if (fd_ < 0)
    {
        NETWORK_ERROR << "socket failed, errno : " << errno;
        exit(-1);
    }

    if (socket_opt_)
    {
        delete socket_opt_;
        socket_opt_ = nullptr;
    }


    loop_->AddEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));
    socket_opt_ = new SocketOpt(fd_);
    socket_opt_->SetReuseAddr(true);
    socket_opt_->SetReusePort(true);
    socket_opt_->BindAddress(addr_);
    socket_opt_->Listen();


}
void Acceptor::Stop()
{
    loop_->DelEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));

}

void Acceptor::OnRead() 
{
    while(true)
    {
        int sock = socket_opt_->Accept(&addr_);
        if (sock > 0)
        {
            if (accept_cb_)
            {
                accept_cb_(sock, addr_);
            }
        }
        else
        {
            // Resource temporarily unavailable (资源暂时不可用)
            // Interrupted system call (系统调用被中断)

            if (errno != EAGAIN && errno != EINTR)
            {
                NETWORK_ERROR << "accept failed, errno :" << errno;
                OnClose();
            }
            break;
        }
    }
}

void Acceptor::OnError(const std::string &msg) 
{
    NETWORK_ERROR << "accept failed, msg :" << msg;
    OnClose();
}

void Acceptor::OnClose() 
{
    Stop();
    Open();
}