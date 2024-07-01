#include "network/TcpClient.h"
#include "network/base/Network.h" 
#include "network/base/SocketOpt.h"


using namespace vdse::network;

TcpClient::TcpClient(EventLoop *loop, const InetAddress &server)
:TcpConnection(loop, -1, InetAddress(), server),
server_addr_(server)
{

}

TcpClient::~TcpClient()
{
    OnClose();
}

void TcpClient::Connect()
{
    loop_ -> RunInLoop([this](){
        ConnectInloop();
    });
}

void TcpClient::ConnectInloop()
{
    // 判断当前是否在loop线程中
    loop_->AssertLoopInThread();
    

    fd_ = SocketOpt::CreatNonblockingTcpSocket(AF_INET);

    if (fd_ < 0)
    {
        OnClose();
        return;
    }

    status_ = kTcpClientStatusConnecting;

    loop_->AddEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));

    EnableWriting(true);

    // EnableCheckIdleTimeOut(3);


    SocketOpt opt(fd_);
    auto ret = opt.Connect(server_addr_);

    if (ret == 0)
    {
        UpdateConnectionStatus();
    }
    else if (ret == -1)
    {
        if (errno != EINPROGRESS)
        {
            NETWORK_ERROR << "Connection to server : "<< server_addr_.ToIpPort() << "failed, errno : " << errno;
            OnClose();
        }

    }
}

void TcpClient::SetConnectCallBack(ConnectionCallBack &&cb)
{
    connection_cb_ = cb;
}

void TcpClient::SetConnectCallBack(const ConnectionCallBack &cb)
{
    connection_cb_ = std::move(cb);
}

void TcpClient::UpdateConnectionStatus()
{
    status_ = kTcpClientStatusConnected;
    if (connection_cb_)
    {
        connection_cb_(std::dynamic_pointer_cast<TcpClient>(shared_from_this()), true);
    }
}

bool TcpClient::CheckError()
{
    int error = 0;
    socklen_t len = sizeof(error);
    ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len);
    return error != 0;
} 


void TcpClient::OnRead() 
{
    if (status_ == kTcpClientStatusConnecting)
    {
        if (CheckError())
        {
            NETWORK_ERROR << "OnRead server : "<< server_addr_.ToIpPort() << "error, errno : " << errno;
            OnClose();
            return;
        }
        UpdateConnectionStatus();
    }
    else if (status_ == kTcpClientStatusConnected)
    {
        TcpConnection::OnRead();
    }
}
void TcpClient::OnWrite() 
{
    if (status_ == kTcpClientStatusConnecting)
    {
        if (CheckError())
        {
            NETWORK_ERROR << "OnWrite server : "<< server_addr_.ToIpPort() << "error, errno : " << errno;
            OnClose();
            return;
        }
        UpdateConnectionStatus();
    }
    else if (status_ == kTcpClientStatusConnected)
    {
        TcpConnection::OnWrite();
    }
}
void TcpClient::OnClose() 
{
    if (status_ == kTcpClientStatusConnected || status_ == kTcpClientStatusConnecting)
    {
        loop_->DelEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    }
    status_ = ktcpClientStatusDisConnected;
    TcpConnection::OnClose();
}


void TcpClient::Send(std::list<BufferNodePtr> &list)
{
    if (status_ == kTcpClientStatusConnected)
    {
        TcpConnection::Send(list);
    }
}
void TcpClient::Send(const char *buf, size_t size)
{
    if (status_ == kTcpClientStatusConnected)
    {
        TcpConnection::Send(buf, size );
    }
}