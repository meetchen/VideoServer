/**
 * @FilePath     : /VideoServer/src/network/TcpServer.h
 * @Description  :  管理所有的TcpConnection
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-29 21:17:59
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#pragma once

#include "network/net/TcpConnection.h"
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include "network/net/Acceptor.h"
#include <unordered_set>


namespace vdse
{
    namespace network
    {   
        using NewConnctionCallBack = std::function<void(const TcpConnectionPtr&)>;
        using DestroyConnectionCallBack = std::function<void(const TcpConnectionPtr&)>;
        class TcpServer
        {
            public:
                TcpServer(EventLoop *loop, const InetAddress &addr);
                virtual ~TcpServer();

                void SetNewConnectionCallBack(const NewConnctionCallBack & cb);
                void SetNewConnecitonCallBack(NewConnctionCallBack &&cb);
    
                void SetDestroyConnectionCallBack(const DestroyConnectionCallBack & cb);
                void SetDestroyConnectionCallBack(DestroyConnectionCallBack &&cb);

                void SetMsgCallBack(const MsgCallBack &cb);
                void SetMsgCallBack(MsgCallBack &&cb);

                void SetActiveCallBack(const ActiveCallBack &cb);
                void SetActiveCallBack(ActiveCallBack &&cb);

                void SetWriteCompleteCallBack(const WriteCompleteCallBack &cb);
                void SetWriteCompleteCallBack(WriteCompleteCallBack &&cb);

                void OnConnectionClose(const TcpConnectionPtr &conn);
                /**
                 * @brief        : acceptor 的调用函数，处理函数 
                 * @param         {int} fd:
                 * @param         {InetAddress} &addr:
                 * @return        {*}
                **/                
                void OnAccept(int fd, const InetAddress &addr);

                virtual void Start();
                virtual void Stop();

            private:
                EventLoop *loop_{nullptr};
                InetAddress addr_;
                std::shared_ptr<Acceptor> acceptor_;
                std::unordered_set<TcpConnectionPtr> connections_;
                NewConnctionCallBack new_connection_cb_;
                DestroyConnectionCallBack destroy_connection_cb_;
                MsgCallBack message_cb_;
                ActiveCallBack active_cb_;
                WriteCompleteCallBack write_complete_cb_;

        };
    }
}