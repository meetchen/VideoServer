/**
 * @FilePath     : /VideoServer/src/network/net/UdpClient.h
 * @Description  :  负责注册事件 删除事件
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 15:00:44
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#pragma once

#include "network/net/UdpSocket.h"


namespace vdse
{
    namespace network
    {
        using ConnectedCallBack = std::function<void(const UdpSocketPtr&, bool)>;
        class UdpClient : public UdpSocket
        {
            public:
                UdpClient(EventLoop *loop, const InetAddress &server);
                virtual ~UdpClient();
                
                void Connect();
                void SetConnectedCallBack(const ConnectedCallBack &cb);
                void SetConnectedCallBack(ConnectedCallBack &&cb);
                void Send(std::list<UdpBufferNodePtr> &list);
                void Send(const char *buf, size_t size);
                void OnClose() override;
            private:
                void ConnectInLoop();
                InetAddress server_addr_;
                struct sockaddr_in6 sock_addr_;
                socklen_t sock_len_{sizeof(struct sockaddr_in6)};
                ConnectedCallBack connectioned_cb_;
                bool connected_{false};
        
        };
    }
}