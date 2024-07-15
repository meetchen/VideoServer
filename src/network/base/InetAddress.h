#pragma once

#include <string>
#include <bits/sockaddr.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <memory>


namespace vdse
{
    namespace network
    {
        class InetAddress
        {
            public:
                InetAddress(const std::string &ip, uint16_t port, bool is_v6 = false);
                InetAddress(const std::string &host, bool is_v6 = false);
                InetAddress() = default;
                ~InetAddress() = default;

                void SetHost(const std::string &host);
                void SetAddr(const std::string &addr);
                void SetPort(uint16_t port);
                void SetIsIPV6(bool is_v6);

               
                const std::string& IP() const;
                uint32_t IPv4() const;
                std::string ToIpPort() const;
                uint16_t Port() const;
                void GetSockAddr(struct sockaddr *saddr) const;

                bool IsIpV6() const;
                bool IsWanIp();
                bool IsLanIp();
                bool IsLoopbackIp();

                 // 传入host 获取ip和端口号
                void static GetIpAndPort(const std::string &host, std::string &ip, std::string &port);
                
                static InetAddress ParseSockAddr(struct sockaddr_in6 * addr);

            private:
                uint32_t IPv4(const char *ip) const;
                std::string ip_;
                std::string port_;
                bool is_v6_{false};
        };

        using InetAddressPtr = std::shared_ptr<InetAddress>;

    }
} 