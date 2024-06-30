#include "network/base/InetAddress.h"
#include "network/base/Network.h"
#include <iostream>
#include <cstring>
#include <sstream>

using namespace vdse::network;

void InetAddress::GetIpAndPort(const std::string &host, std::string &ip, std::string &port)
{
    auto index = host.find_first_of(':', 0);
    if (index != host.npos)
    {
        ip = host.substr(0, index);
        port = host.substr(index + 1);
    }
    else
    {
        ip = host;
    }
}

InetAddress::InetAddress(const std::string &ip, uint16_t port, bool is_v6)
    : ip_(ip),
      port_(std::to_string(port)),
      is_v6_(is_v6)
{
}

InetAddress::InetAddress(const std::string &host, bool is_v6)
{
    is_v6_ = is_v6;
    GetIpAndPort(host, ip_, port_);
}

void InetAddress::SetHost(const std::string &host)
{
    GetIpAndPort(host, ip_, port_);
}

void InetAddress::SetAddr(const std::string &addr)
{
    ip_ = addr;
}

void InetAddress::SetPort(uint16_t port)
{
    port_ = std::to_string(port);
}

void InetAddress::SetIsIPV6(bool is_v6)
{
    is_v6_ = is_v6;
}

const std::string &InetAddress::IP() const
{
    return ip_;
}

uint32_t InetAddress::IPv4(const char *ip) const
{
    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = 0;

    if (::inet_pton(AF_INET, ip, &addr.sin_addr) < 0)
    {
        NETWORK_ERROR << "Ipv4 ip :" << ip << "Convert failed";
    }
    // Network To Host Long
    // 网络字节序 转为 主机字节序
    return ntohl(addr.sin_addr.s_addr);
}

uint32_t InetAddress::IPv4() const
{
    return IPv4(ip_.c_str());
}

std::string InetAddress::ToIpPort() const
{
    std::stringstream ss;
    ss << ip_ << ":" << port_;
    return ss.str();
}

uint16_t InetAddress::Port() const
{
    // 将一个字符串形式的端口号转换为整数
    return std::atoi(port_.c_str());
}

void InetAddress::GetSockAddr(struct sockaddr *saddr) const
{
    if (is_v6_)
    {
        struct sockaddr_in6 *addr_in6 = reinterpret_cast<struct sockaddr_in6 *>(saddr);
        memset(&addr_in6, 0x00, sizeof(addr_in6));

        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port =  htons(std::atoi(port_.c_str()));

        if (::inet_pton(AF_INET6, ip_.c_str(), &addr_in6->sin6_addr) < 0)
        {
        }

        return;
    }

    struct sockaddr_in *addr_in = reinterpret_cast<struct sockaddr_in *>(saddr);
    memset(addr_in, 0x00, sizeof(struct sockaddr_in));

    addr_in->sin_family = AF_INET;
    addr_in->sin_port =  htons(std::atoi(port_.c_str()));

    if (::inet_pton(AF_INET, ip_.c_str(), &addr_in->sin_addr) < 0)
    {

    }
}

bool InetAddress::IsIpV6() const
{
    return is_v6_;
}

bool InetAddress::IsWanIp()
{
    uint32_t a_start = IPv4("10.0.0.0");
    uint32_t a_end = IPv4("10.255.255.255");

    uint32_t b_start = IPv4("172.16.0.0");
    uint32_t b_end = IPv4("172.31.255.255");

    uint32_t c_start = IPv4("192.168.0.0");
    uint32_t c_end = IPv4("192.168.255.255");

    uint32_t ip = IPv4();

    bool is_a = ip >= a_start && ip <= a_end;
    bool is_b = ip >= b_start && ip <= b_end;
    bool is_c = ip >= c_start && ip <= c_end;

    return !is_a && !is_b && !is_c && ip != INADDR_LOOPBACK;
}

bool InetAddress::IsLanIp()
{
    uint32_t a_start = IPv4("10.0.0.0");
    uint32_t a_end = IPv4("10.255.255.255");

    uint32_t b_start = IPv4("172.16.0.0");
    uint32_t b_end = IPv4("172.31.255.255");

    uint32_t c_start = IPv4("192.168.0.0");
    uint32_t c_end = IPv4("192.168.255.255");

    uint32_t ip = IPv4();

    bool is_a = ip >= a_start && ip <= a_end;
    bool is_b = ip >= b_start && ip <= b_end;
    bool is_c = ip >= c_start && ip <= c_end;

    return is_a || is_b || is_c;
}

bool InetAddress::IsLoopbackIp()
{
    return ip_ == "127.0.0.1";
}

InetAddress InetAddress::ParseSockAddr(struct sockaddr_in6 * addr)
{
    InetAddress inet_addr;

    if (addr->sin6_family == AF_INET)
    {
        char ip[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in *temp = (struct sockaddr_in*)addr;
        ::inet_ntop(AF_INET, &(temp->sin_addr.s_addr), ip, INET_ADDRSTRLEN);
        inet_addr.SetAddr(ip);
        inet_addr.SetPort(ntohs(temp->sin_port));
    }
    else if (addr->sin6_family == AF_INET6)
    {
        char ip[INET6_ADDRSTRLEN] = {0};

        ::inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);
        inet_addr.SetAddr(ip);
        inet_addr.SetPort(ntohs(addr->sin6_port));
        inet_addr.SetIsIPV6(true);
    }
    return inet_addr;
}