#include "network/net/TcpConnection.h"
#include "network/base/Network.h"

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
            auto ret = ::writev(fd_, &io_vec_list_[0], io_vec_list_.size());
            if (ret >= 0)
            {
                while (ret > 0)
                {
                    
                }
            }
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

}

void TcpConnection::Send(const char *buf, size_t size)
{

}

void TcpConnection::SendInLoop(std::list<BufferNodePtr> &list)
{

}

void TcpConnection::SendInLoop(const char *buf, size_t size)
{

}