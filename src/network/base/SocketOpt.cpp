#include "network/base/SocketOpt.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>

using namespace vdse::network;

SocketOpt::SocketOpt(int sock, bool v6)
    : sock_(sock),
      is_v6_(v6)
{

}

int SocketOpt::CreatNonblockingTcpSocket(int family)
{
    /**
     * SOCK_CLOEXEC 标志用于在创建套接字时设置 close-on-exec (FD_CLOEXEC) 标志。
     * 当使用 exec 系列函数（如 execl, execv, execle, execve, execlp, execvp 等）执行新程序时，
     * 设置了 FD_CLOEXEC 标志的文件描述符将被自动关闭。
     * 这可以防止文件描述符泄露到新执行的程序中，提升程序的安全性和稳定性。
     */
    return ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
}

int SocketOpt::CreatNonblockingUdpSocket(int family)
{
    return ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
}

int SocketOpt::BindAddress(const InetAddress &local_addr)
{
    if (local_addr.IsIpV6())
    {
        struct sockaddr_in6 addr6;
        local_addr.GetSockAddr((struct sockaddr *)&addr6);
        return ::bind(sock_, (struct sockaddr *)&addr6, sizeof(addr6));
    }
    else
    {
        struct sockaddr_in addr;
        local_addr.GetSockAddr((struct sockaddr *)&addr);
        return ::bind(sock_, (struct sockaddr *)&addr, sizeof(addr));
    }

}

int SocketOpt::Listen()
{
    // SOMAXCONN 是一个常量，用于指定在调用 listen 函数时，操作系统内核允许的最大连接队列长度
    return ::listen(sock_, SOMAXCONN);
}

int SocketOpt::Accept(InetAddress *perr_addr)
{
    struct sockaddr_in6 addr6;
    socklen_t len = sizeof(struct sockaddr_in6);

    int sock = ::accept4(sock_,  (struct sockaddr *)&addr6, &len, SOCK_CLOEXEC | SOCK_NONBLOCK);

    if (sock <= 0) return sock;

    if (addr6.sin6_family == AF_INET)
    {
        char ip[INET_ADDRSTRLEN] = {0};
        // 将原本假定为ipv6的结构体转化为ipv4 ！！！
        struct sockaddr_in *addr = (struct sockaddr_in*)&addr6;
        ::inet_ntop(AF_INET, &(addr->sin_addr.s_addr), ip, INET_ADDRSTRLEN);
        perr_addr->SetAddr(ip);
        perr_addr->SetPort(ntohs(addr->sin_port));

    }
    else if (addr6.sin6_family == AF_INET6)
    {
        char ip[INET6_ADDRSTRLEN] = {0};

        ::inet_ntop(AF_INET6, &(addr6.sin6_addr), ip, INET6_ADDRSTRLEN);
        perr_addr->SetAddr(ip);
        perr_addr->SetPort(ntohs(addr6.sin6_port));
        perr_addr->SetIsIPV6(true);
    }


    return sock;
}


int SocketOpt::Connect(const InetAddress &addr)
{

    struct sockaddr_in6 addr6;

    addr.GetSockAddr((struct sockaddr *)&addr6);

    return ::connect(sock_, (struct sockaddr *)&addr6, sizeof(struct sockaddr_in6));

}


InetAddressPtr SocketOpt::GetLocalAddr()
{
    struct sockaddr_in6 saddr;
    socklen_t len = sizeof(struct sockaddr_in6);

    ::getsockname(sock_, (struct sockaddr *)&saddr, &len);

    InetAddressPtr perr_addr = std::make_shared<InetAddress>();

    if (saddr.sin6_family == AF_INET)
    {
        char ip[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in *addr = (struct sockaddr_in*)&saddr;
        ::inet_ntop(AF_INET, &(addr->sin_addr.s_addr), ip, INET_ADDRSTRLEN);
        perr_addr->SetAddr(ip);
        perr_addr->SetPort(ntohs(addr->sin_port));

    }
    else if (saddr.sin6_family == AF_INET6)
    {
        char ip[INET6_ADDRSTRLEN] = {0};

        ::inet_ntop(AF_INET6, &(saddr.sin6_addr), ip, INET6_ADDRSTRLEN);
        perr_addr->SetAddr(ip);
        perr_addr->SetPort(ntohs(saddr.sin6_port));
        perr_addr->SetIsIPV6(true);
    }
    
    return perr_addr;
}

InetAddressPtr SocketOpt::GetPeerAddr()
{
    struct sockaddr_in6 saddr;
    socklen_t len = sizeof(struct sockaddr_in6);
    ::getpeername(sock_, (struct sockaddr *)&saddr, &len);

    InetAddressPtr perr_addr = std::make_shared<InetAddress>();

     if (saddr.sin6_family == AF_INET)
    {
        char ip[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in *addr = (struct sockaddr_in*)&saddr;
        ::inet_ntop(AF_INET, &(addr->sin_addr.s_addr), ip, INET_ADDRSTRLEN);
        perr_addr->SetAddr(ip);
        perr_addr->SetPort(ntohs(addr->sin_port));
    }
    else if (saddr.sin6_family == AF_INET6)
    {
        char ip[INET6_ADDRSTRLEN] = {0};

        ::inet_ntop(AF_INET6, &(saddr.sin6_addr), ip, INET6_ADDRSTRLEN);
        perr_addr->SetAddr(ip);
        perr_addr->SetPort(ntohs(saddr.sin6_port));
        perr_addr->SetIsIPV6(true);

        // std::cout << "ip = " << ip << " port = " << perr_addr->Port() << std::endl;
    }
    return perr_addr;
}


void SocketOpt::SetTcpNoDelay(bool on)
{
    int optValue = on ? 1 : 0;
    ::setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, &optValue, sizeof(optValue));
}

void SocketOpt::SetReuseAddr(bool on)
{
    int optValue = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &optValue, sizeof(optValue));
}

void SocketOpt::SetReusePort(bool on)
{
    int optValue = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_REUSEPORT, &optValue, sizeof(optValue));
}

void SocketOpt::SetKeepAlive(bool on)
{
    int optValue = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_KEEPALIVE, &optValue, sizeof(optValue));
}

void SocketOpt::SetNonBlocking(bool on)
{
    int flag = ::fcntl(sock_, F_GETFL, 0);
    if (on)
    {
        flag |= O_NONBLOCK;
    }
    else
    {
        flag &= ~O_NONBLOCK;
    }
    ::fcntl(sock_, F_SETFL, flag);
}