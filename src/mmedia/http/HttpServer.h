#pragma once
#include "mmedia/http/HttpCallBack.h"
#include "network/net/TcpConnection.h"
#include "network/TcpServer.h"

namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        class HttpServer:public TcpServer
        {
        public:
            HttpServer(EventLoop *loop,const InetAddress &local,HttpCallBack *handler=nullptr);
            ~HttpServer();

            void Start() override;
            void Stop() override;

        private:
            void OnNewConnection(const TcpConnectionPtr &conn);
            void OnDestroyed(const TcpConnectionPtr &conn);
            void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf);
            void OnWriteComplete(const ConnectionPtr &con);
            void OnActive(const ConnectionPtr &conn);
            HttpCallBack *http_handler_{nullptr};
        };
    }
}