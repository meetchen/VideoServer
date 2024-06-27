#include "network/net/TcpConnection.h"
#include "network/base/Network.h"
#include <unistd.h>

using namespace vdse::network;


TcpConnection::TcpConnection(EventLoop *loop, int socketfd, const InetAddress& localAddr, const InetAddress& peerAddr)
:Connection(loop, socketfd, localAddr, peerAddr)
{

}

TcpConnection::~TcpConnection()
{
    OnClose();
}

void TcpConnection::OnClose()
{
    loop_ ->AssertLoopInThread();
    if (!closed_)
    {
        if (close_cb_)
        {
            close_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
        closed_ = true;
    }
}

void TcpConnection::ForceClose() 
{
    loop_->RunInLoop([this](){
        OnClose();
    });
}

void TcpConnection::SetCloseCallBack(const CloseConnectionCallBack & cb)
{
    close_cb_ = cb;
}

void TcpConnection::SetCloseCallBack(CloseConnectionCallBack && cb)
{
    close_cb_ = std::move(cb);
}

void TcpConnection::SetRecvMsgCallBack(const MsgCallBack &cb)
{
    msg_cb_ = cb;
}

void TcpConnection::SetRecvMsgCallBack(MsgCallBack &&cb)
{
    msg_cb_ = std::move(cb);
}

void TcpConnection::OnRead()
{
    if (closed_)
    {
        NETWORK_ERROR << "host : " << peer_addr_.ToIpPort() << " had closed .";
        return;
    }

    while (true)
    {
        int err = 0;
        auto ret = msg_buf_.ReadFd(fd_, &err);
        if (ret > 0)
        {
            if (msg_cb_)
            {
                msg_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()), msg_buf_);
            }
        }
        else if (ret == 0)
        {
            OnClose();
            break;
        }
        else
        {
            if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK)
            {
                NETWORK_ERROR << "read err : " << err;
                OnClose();
            }
        }

    }
}

void TcpConnection::OnError(const std::string &msg) 
{
    NETWORK_ERROR << "host : " << peer_addr_.ToIpPort() << "error, msg : " << msg;
    OnClose();
}

void TcpConnection::OnWrite()
{
    if (closed_)
    {
        NETWORK_ERROR << "host : " << peer_addr_.ToIpPort() << " had closed .";
        return;
    }

    if (!io_vec_list_.empty())
    {
        while (true)
        {
            // writev 一次将多个非连续内存的地址写入到文件描述符
            // ret 返回写入成功的字节数， 出错返回-1，并设置errno
            auto ret = ::writev(fd_, &io_vec_list_[0], io_vec_list_.size());
            if (ret >= 0)
            {
                while (ret > 0)
                {
                    if (io_vec_list_.front().iov_len > ret)
                    {
                        io_vec_list_.front().iov_base = (char *)io_vec_list_.front().iov_base + ret;
                        io_vec_list_.front().iov_len -= ret;
                        break;
                    }       
                    else
                    {
                        ret -= io_vec_list_.front().iov_len;
                        io_vec_list_.erase(io_vec_list_.begin());
                    }
                }
            
            }
            if (io_vec_list_.empty())
            {
                EnableWriting(false);
                write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
            }
        }
    }
    else
    {
        if (io_vec_list_.empty())
        {
            EnableWriting(false);
            write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
    }
}

void TcpConnection::SetWriteCompleteCallBack(const WriteCompleteCallBack &cb)
{
    write_complete_cb_ = cb;
}

void TcpConnection::SetWriteCompleteCallBack(WriteCompleteCallBack &&cb)
{
    write_complete_cb_ = std::move(cb);
}

void TcpConnection::Send(std::list<BufferNodePtr> &list)
{
    // &list 引用捕获x
    loop_->RunInLoop([this, &list](){
        SendInLoop(list);
    });
}

void TcpConnection::Send(const char *buf, size_t size)
{
    loop_->RunInLoop([this, buf, size](){
        SendInLoop(buf, size);
    });
}

void TcpConnection::SendInLoop(std::list<BufferNodePtr> &list)
{
    if (closed_)
    {
        NETWORK_ERROR << "host : " << peer_addr_.ToIpPort() << " had closed .";
        return;
    }

    for (auto &buf : list)
    {
        struct iovec vec;
        vec.iov_base = (void *)buf->addr;
        vec.iov_len = buf->size;
        io_vec_list_.push_back(vec);
    }
    if (!io_vec_list_.empty())
    {
        EnableWriting(true);
    }
}

void TcpConnection::SendInLoop(const char *buf, size_t size)
{
    if (closed_)
    {
        NETWORK_ERROR << "host : " << peer_addr_.ToIpPort() << " had closed .";
        return;
    }

    size_t len = 0;

    if (io_vec_list_.empty())
    {
        len = ::write(fd_, buf, size);
        if (len < 0)
        {
            if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                NETWORK_ERROR << "read err : " << errno;
                OnClose();
            }
            len = 0;
        }
        size -= len;

    }
    if (size > 0)
    {
        struct iovec vec;
        vec.iov_base = (void *)(buf + len);
        vec.iov_len = size;
        io_vec_list_.push_back(vec);
        EnableWriting(true);
    }
}

void TcpConnection::ExtendLife()
{
    auto tp = timeout_entry_.lock();
    if (tp)
    {   
        // 增加了timeout entry 的引用计数，只有在最后定时任务到期的时候才会触发析构， 
        loop_->InsertEntry(max_idle_time_, tp);
    }
}

void TcpConnection::OnTimeOut()
{ 
    NETWORK_ERROR << "host " << peer_addr_.ToIpPort() << "time out and closed.."; 
    OnClose();
}

void TcpConnection::EnableCheckIdleTimeOut(int32_t max_time)
{
    auto tp = std::make_shared<TimeOutEntry>(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
    max_idle_time_ = max_time;
    timeout_entry_ = tp;
    loop_->InsertEntry(max_time, tp);
}

void TcpConnection::SetTimeOutCallBack(int timeout, const TimeOutCallBack &cb)
{
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
    loop_->RunAfter(timeout, [&cp, &cb](){
        cb(cp);
    });
}
void TcpConnection::SetTimeOutCallBack(int timeout, TimeOutCallBack &&cb)
{
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
    loop_->RunAfter(timeout, [&cp, cb](){
        cb(cp);
    });
}