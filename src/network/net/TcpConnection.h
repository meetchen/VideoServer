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
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

        struct BufferNode
        {
            BufferNode(void *buf, size_t len)
            :addr(buf), size(len)
            {

            }
            void *addr{nullptr};
            size_t size{0};
        };
        using BufferNodePtr = std::shared_ptr<BufferNode>;

        using CloseConnectionCallBack = std::function<void(const TcpConnectionPtr&)>;
        using MsgCallBack = std::function<void(const TcpConnectionPtr&, MsgBuffer &)>;
        using WriteCompleteCallBack = std::function<void(const TcpConnectionPtr&)>;

        

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

                void SetCloseCallBack(const CloseConnectionCallBack & cb);
                void SetCloseCallBack(CloseConnectionCallBack &&cb);

                void SetRecvMsgCallBack(const MsgCallBack &cb);
                void SetRecvMsgCallBack(MsgCallBack &&cb);

                void SetWriteCompleteCallBack(const WriteCompleteCallBack &cb);
                void SetWriteCompleteCallBack(WriteCompleteCallBack &&cb);

                void Send(std::list<BufferNodePtr> &list);
                void Send(const char *buf, size_t size);


            private:
                bool closed_{false};
                CloseConnectionCallBack  close_cb_;
                MsgBuffer msg_buf_;
                MsgCallBack msg_cb_;
                WriteCompleteCallBack write_complete_cb_;
                std::vector<struct iovec> io_vec_list_;

                void SendInLoop(std::list<BufferNodePtr> &list);
                void SendInLoop(const char *buf, size_t size);

        };
    }
}

 