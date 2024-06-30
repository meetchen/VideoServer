/**
 * @FilePath     : /VideoServer/src/network/net/UdpSocket.cpp
 * @Description  :  imp UdpSocket
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 17:22:27
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/net/UdpSocket.h"
#include "network/base/Network.h"

#include <iostream>

using namespace vdse::network;


UdpSocket::UdpSocket(EventLoop *loop, int socketfd, const InetAddress &localAddr, const InetAddress &peerAddr)
:Connection(loop, socketfd, localAddr, peerAddr), message_buffer_(message_buffer_size_)
{

}

UdpSocket::~UdpSocket()
{

}

void UdpSocket::SetCloseConnectionCallBack(const UdpSocketCloseConnectionCallBack &cb)
{
    close_cb_ = cb;
}

void UdpSocket::SetCloseConnectionCallBack(UdpSocketCloseConnectionCallBack &&cb)
{
    close_cb_ = std::move(cb);
}

void UdpSocket::SetTimeOutCallBack(int timeout, const UdpSocketTimeOutCallBack &cb)
{
    auto us = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());
    loop_->RunAfter(timeout, [this, cb, us](){
        cb(us);
    });
}

void UdpSocket::SetTimeOutCallBack(int timeout, UdpSocketTimeOutCallBack &&cb)
{
    auto us = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());
    loop_->RunAfter(timeout, [this, cb, us](){
        cb(us);
    });
}

void UdpSocket::SetRecvMsgCallBack(const UdpSocketRecvMessageCallBack &cb)
{
    message_cb_ = cb;
}
void UdpSocket::SetRecvMsgCallBack(UdpSocketRecvMessageCallBack &&cb)
{
    message_cb_ = std::move(cb);
}

void UdpSocket::SetWriteCompleteCallBack(const UdpSocketWriteCompleteCallBack &cb)
{
    write_complete_cb_ = cb;
}
void UdpSocket::SetWriteCompleteCallBack(UdpSocketWriteCompleteCallBack &&cb)
{
    write_complete_cb_ = std::move(cb);
}

void UdpSocket::ExtendLife()
{
    auto tp = timeout_entry_.lock();
    if (tp)
    {   
        // 增加了timeout entry 的引用计数，只有在最后定时任务到期的时候才会触发析构， 
        loop_->InsertEntry(max_idle_time_, tp);
    }
}

void UdpSocket::EnableCheckIdleTimeOut(int32_t max_time)
{
    auto tp = std::make_shared<UdpTimeOutEntry>(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
    max_idle_time_ = max_time;
    timeout_entry_ = tp;
    loop_->InsertEntry(max_time, tp);
}

void UdpSocket::OnTimeOut()
{
    NETWORK_TRACE << " Host : " << peer_addr_.ToIpPort() << " Time out and closed";
    OnClose();
}


void UdpSocket::OnRead()
{

    if (closed_)
    { 
        NETWORK_ERROR << " Host : " << peer_addr_.ToIpPort() << " has closed ";
        return;
    }
    ExtendLife();
    while (true)
    {
        struct sockaddr_in6 sock_addr;
        socklen_t len = sizeof(struct sockaddr_in6);
        auto ret = ::recvfrom(fd_, message_buffer_.BeginWrite(), message_buffer_size_, 0,(struct sockaddr *)&sock_addr, &len);
        if (ret > 0)
        { 
            auto peerAddr = InetAddress::ParseSockAddr(&sock_addr);
            message_buffer_.HasWritten(ret);
            if (message_cb_)
            {
                message_cb_(peerAddr, message_buffer_);
            }
            message_buffer_.RetrieveAll();
        }
        else if (ret < 0)
        {

            if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                NETWORK_ERROR << "host: " << peer_addr_.ToIpPort() << " error : " << errno;
                OnClose();
                // 等待下一次读事件的触发
                break;
            }
        }

    }
}

void UdpSocket::OnWrite()
{
    if (closed_)
    {
        NETWORK_ERROR << " Host : " << peer_addr_.ToIpPort() << " has closed";
        return;
    }
    ExtendLife();
    while (true)
    {
        if (!buffer_list_.empty())
        {
            auto buf = buffer_list_.front();
            auto ret = ::sendto(fd_, buf->addr, buf->size, 0, buf->sock_addr, buf->sock_len);
            if (ret > 0)
            {
                buffer_list_.pop_front();
            }
            else if (ret < 0)
            {
                if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    NETWORK_ERROR << "host: " << peer_addr_.ToIpPort() << " error : " << errno;
                    OnClose();
                    // 等待下一次事件的触发
                    break;
                }
            }
        }
        // 这块不能else 可能上面处理完了 就空了
        if (buffer_list_.empty())
        {
            if (write_complete_cb_)
            {
                write_complete_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
            }
            break;
        }
    }


}

void UdpSocket::OnClose() 
{
    if (!closed_)
    {
        closed_ = true;
        if (close_cb_)
        {
            close_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        }
        Event::Close();
    }
}

void UdpSocket::OnError(const std::string &msg) 
{
    NETWORK_ERROR << " Host : " << peer_addr_.ToIpPort() << " error and closed";
    OnClose();
}

void UdpSocket::SendInLoop(std::list<UdpBufferNodePtr> &list)
{
    for (auto&l : list)
    {
        buffer_list_.push_front(l);
    }

    if (!buffer_list_.empty())
    {
        EnableWriting(true);
    }

}

void UdpSocket::SendInLoop(const char *buf, size_t size, struct sockaddr *addr, socklen_t len)
{
    if (buffer_list_.empty())
    {
        auto ret = ::sendto(fd_, buf, size, 0, addr, len);
        if (ret > 0)
        {
            return;
        }
    }
    auto node = std::make_shared<UdpBufferNode>((void *)buf, size, addr,len);
    buffer_list_.emplace_back(node);
    EnableWriting(true);
}

void UdpSocket::Send(std::list<UdpBufferNodePtr> &list)
{
    loop_->RunInLoop([this, &list](){
        SendInLoop(list);
    });
}

void UdpSocket::Send(const char *buf, size_t size, struct sockaddr *addr, socklen_t len)
{
    loop_->RunInLoop([this, buf, size, addr, len](){
        SendInLoop(buf,size,addr,len);
    });
}

void UdpSocket::ForceClose() 
{
    loop_->RunInLoop([this](){
        OnClose();
    });
}