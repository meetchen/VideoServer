#include "network/net/EventLoop.h"
#include "network/base/SocketOpt.h"
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"

namespace vdse
{
    namespace network
    {
        using AcceptCallBack = std::function<void(int sock,const InetAddress &addr)>;

        class Acceptor : public Event
        {
            public:
                Acceptor(EventLoop *loop, const InetAddress &addr);
                ~Acceptor();

                void SetAcceptCallBack(const AcceptCallBack &cb);
                void SetAcceptCallBack(AcceptCallBack &&cb);

                void Start();
                void Stop();
                void OnRead() override;
                void OnError(const std::string &msg) override;
                void OnClose() override;
            private:
                void Open();
                InetAddress addr_;
                AcceptCallBack accept_cb_;
                SocketOpt *socket_opt_{nullptr};

        };   
    }
}



