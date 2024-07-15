/**
 * @FilePath     : /VideoServer/src/network/net/UdpSocket.h
 * @Description  :  负责处理UDP的IO
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 18:35:09
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#pragma once
#include "network/net/Connection.h"
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include "network/base/MsgBuffer.h"
#include <list>
#include <functional>
#include <memory>


namespace vdse
{
    namespace network
    {

        class UdpSocket;
        struct UdpTimeOutEntry;

        using UdpSocketPtr = std::shared_ptr<UdpSocket>;

        using UdpSocketRecvMessageCallBack = std::function<void(const InetAddress &addr, MsgBuffer & buff)>;

        using UdpSocketWriteCompleteCallBack = std::function<void(const UdpSocketPtr&)>;

        using UdpSocketTimeOutCallBack = std::function<void(const UdpSocketPtr&)>;
        
        using UdpSocketCloseConnectionCallBack = std::function<void(const UdpSocketPtr&)>;
        



        struct UdpBufferNode: public BufferNode
        {
            UdpBufferNode(void *buf, size_t len, struct sockaddr * saddr, socklen_t size)
            :BufferNode(buf, len),sock_addr(saddr), sock_len(size)
            {

            }
            struct sockaddr *sock_addr{nullptr};
            socklen_t sock_len{0};
        };
        using UdpBufferNodePtr = std::shared_ptr<UdpBufferNode>;


        class UdpSocket : public Connection
        {
            public:
                UdpSocket(EventLoop *loop, int socketfd, const InetAddress &localAddr, const InetAddress &peerAddr);
                ~UdpSocket();

                void SetCloseConnectionCallBack(const UdpSocketCloseConnectionCallBack &cb);
                void SetCloseConnectionCallBack(UdpSocketCloseConnectionCallBack &&cb);

                void SetTimeOutCallBack(int timeout, const UdpSocketTimeOutCallBack &cb);
                void SetTimeOutCallBack(int timeout, UdpSocketTimeOutCallBack &&cb);
                
                void SetWriteCompleteCallBack(const UdpSocketWriteCompleteCallBack &cb);
                void SetWriteCompleteCallBack(UdpSocketWriteCompleteCallBack &&cb);

                void SetRecvMsgCallBack(const UdpSocketRecvMessageCallBack &cb);
                void SetRecvMsgCallBack(UdpSocketRecvMessageCallBack &&cb);

                void OnTimeOut();
                void OnRead() override;
                void OnWrite() override;
                void OnClose() override;
                void OnError(const std::string &msg) override;

                void Send(std::list<UdpBufferNodePtr> &list);
                void Send(const char *buf, size_t size, struct sockaddr *addr, socklen_t len);

                void ForceClose() override;


            private:

                
                void ExtendLife();
                /**
                 * @brief        : 启动闲时检测 
                 * @param         {int32_t} max_time:
                 * @return        {*}
                **/                
                void EnableCheckIdleTimeOut(int32_t max_time);

                void SendInLoop(std::list<UdpBufferNodePtr> &list);
                void SendInLoop(const char *buf, size_t size, struct sockaddr *addr, socklen_t len);


                // 发送数据暂存区
                std::list<UdpBufferNodePtr> buffer_list_;
                bool closed_{false};
                int32_t max_idle_time_{30};
                std::weak_ptr<TimeOutEntry<UdpSocket>>  timeout_entry_;
                MsgBuffer message_buffer_;
                int32_t message_buffer_size_{65535};
                // 接收到消息的回调
                UdpSocketRecvMessageCallBack message_cb_;
                UdpSocketCloseConnectionCallBack close_cb_;
                UdpSocketWriteCompleteCallBack write_complete_cb_;
                UdpSocketTimeOutCallBack time_out_cb;
        };



    }
}
