#pragma once

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <cstdlib>
#include "network/base/InetAddress.h"
#include <memory>

namespace vdse
{
    namespace network
    {   
        class SocketOpt
        {
            public:
                SocketOpt(int sock, bool v6 = false);
                ~SocketOpt() = default;
                
                static int CreatNonblockingTcpSocket(int family);
                static int CreatNonblockingUdpSocket(int family);

                int BindAddress(const InetAddress &localaddr);
                int Listen();
                int Accept(InetAddress *peeraddr);

                int Connect(const InetAddress &dist_addr);

                InetAddressPtr GetLocalAddr();
                InetAddressPtr GetPeerAddr();

                void SetTcpNoDelay(bool on);
                void SetReuseAddr(bool on);
                void SetReusePort(bool on);
                void SetKeepAlive(bool on);
                void SetNonBlocking(bool on);

            private:
                int sock_{-1};
                bool is_v6_{false};
        };
    }
}