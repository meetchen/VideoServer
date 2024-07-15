#pragma once


#include "network/net/Connection.h"
#include "network/base/InetAddress.h"
#include "network/base/MsgBuffer.h"
#include <functional>
#include <list>
#include <sys/uio.h>
#include <memory>


namespace vdse
{
    namespace network
    {   
        class TcpConnection;
        struct TcpTimeOutEntry;
        
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

        using CloseConnectionCallBack = std::function<void(const TcpConnectionPtr&)>;
        using MsgCallBack = std::function<void(const TcpConnectionPtr&, MsgBuffer &)>;
        using WriteCompleteCallBack = std::function<void(const TcpConnectionPtr&)>;
        using TimeOutCallBack = std::function<void(const TcpConnectionPtr&)>;

        using BufferNodePtr = std::shared_ptr<BufferNode>;


        class TcpConnection : public Connection
        {
            public:
                TcpConnection(EventLoop *loop, int socketfd, const InetAddress& localAddr, const InetAddress& peerAddr);
                ~TcpConnection();


                void OnClose() override;
                void ForceClose() override;
                void OnRead() override;
                void OnError(const std::string &msg) override;
                void OnWrite() override;
                void OnTimeOut(); 

                void SetCloseCallBack(const CloseConnectionCallBack & cb);
                void SetCloseCallBack(CloseConnectionCallBack &&cb);

                void SetRecvMsgCallBack(const MsgCallBack &cb);
                void SetRecvMsgCallBack(MsgCallBack &&cb);

                void SetWriteCompleteCallBack(const WriteCompleteCallBack &cb);
                void SetWriteCompleteCallBack(WriteCompleteCallBack &&cb);

                void SetTimeOutCallBack(int timeout, const TimeOutCallBack &cb);
                void SetTimeOutCallBack(int timeout, TimeOutCallBack &&cb); 

                void Send(std::list<BufferNodePtr> &list);
                void Send(const char *buf, size_t size);


                void EnableCheckIdleTimeOut(int32_t max_time);


            private:
                bool closed_{false};
                CloseConnectionCallBack  close_cb_;
                MsgBuffer msg_buf_;
                MsgCallBack msg_cb_;
                WriteCompleteCallBack write_complete_cb_;
                std::vector<struct iovec> io_vec_list_;
                std::weak_ptr<TimeOutEntry<TcpConnection>> timeout_entry_;
                // 默认超时时间 以秒为单位
                uint32_t max_idle_time_{30};

                void SendInLoop(std::list<BufferNodePtr> &list);
                void SendInLoop(const char *buf, size_t size);

                // 延长超时时间
                void ExtendLife();

        };

    }
}

 