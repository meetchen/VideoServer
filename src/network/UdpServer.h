/**
 * @FilePath     : /VideoServer/src/network/UdpServer.h
 * @Description  :  接受不同客户端的数据，通过客户端的IP和端口号区别数据包, 事件注册 删除
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 17:39:07
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#pragma once

#include "network/net/EventLoop.h"
#include "network/base/InetAddress.h"
#include "network/base/SocketOpt.h"
#include "network/net/UdpSocket.h"


namespace vdse
{
    namespace network
    {
        class UdpServer : public UdpSocket
        {
            public:
                UdpServer(EventLoop *loop, const InetAddress &server);
                virtual ~UdpServer();
            
                void Start();
                void Stop();
            
            private:
                void Open();
                InetAddress server_;
        };
    }
}
