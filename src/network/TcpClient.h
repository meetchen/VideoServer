#pragma once

#include "network/net/TcpConnection.h"
#include "network/net/EventLoop.h"
#include "network/base/InetAddress.h"
#include <functional>


namespace vdse
{
    namespace network
    {   
        using ConnectionCallBack = std::function<void(const TcpConnectionPtr&, bool)>;

        enum
        {
            kTcpClientStatusInit = 0,
            kTcpClientStatusConnecting = 1,
            kTcpClientStatusConnected = 2,
            ktcpClientStatusDisConnected = 3,
        };

        class TcpClient : public TcpConnection
        {
            public:
                TcpClient(EventLoop *loop, const InetAddress &server);
                virtual ~TcpClient();

                void OnRead() override;
                void OnWrite() override;
                void OnClose() override;

                void Send(std::list<BufferNodePtr> &list);
                void Send(const char *buf, size_t size);

                void Connect();
                void SetConnectCallBack(ConnectionCallBack &&cb);
                void SetConnectCallBack(const ConnectionCallBack &cb);
            private:
                uint32_t status_{kTcpClientStatusInit};
                ConnectionCallBack connection_cb_;
                InetAddress server_addr_;

                void ConnectInloop();
                bool CheckError(); 
                void UpdateConnectionStatus();



        };

    }
}
