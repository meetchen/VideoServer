#include "network/base/InetAddress.h"
#include <iostream>

using namespace vdse::network;


void TestNormal()
{
    std::string host;
    while(std::cin >> host)
    {
        InetAddress addr(host);

        std::cout << "host:" << host << std::endl
                << " ip:" << addr.IP() << std::endl
                << " port:" << addr.Port() << std::endl
                << " lan:" << addr.IsLanIp() << std::endl
                << " wan:" << addr.IsWanIp() << std::endl
                << " loop:" << addr.IsLoopbackIp() << std::endl;
    }
}

void TestParse()
{
    struct sockaddr_in addr_ipv4;
    struct sockaddr_in6 addr_ipv6;

    // 测试 IPv4 地址
    addr_ipv4.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.1.1", &(addr_ipv4.sin_addr));
    addr_ipv4.sin_port = htons(8080);


    InetAddress result_ipv4 = InetAddress::ParseSockAddr((struct sockaddr_in6 *)&addr_ipv4);

    std::cout << result_ipv4.ToIpPort() << std::endl;

    // 测试 IPv6 地址
    addr_ipv6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "::1", &(addr_ipv6.sin6_addr));
    addr_ipv6.sin6_port = htons(9090);

    InetAddress result_ipv6 = InetAddress::ParseSockAddr(&addr_ipv6);

    std::cout << result_ipv6.ToIpPort() << std::endl;


}

int main(int argc, char const *argv[])
{
    TestParse();
    return 0;
}
