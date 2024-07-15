#pragma once

#include "network/net/Event.h"
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include <unordered_map>
#include <atomic>
#include <functional>


namespace vdse
{
    namespace network
    {   
        enum
        {
            kNormalContext = 0,
            kRtmpContext,
            kHttpContext,
            kUserContext,
            kFlvContext,
        };

        using ContextPtr = std::shared_ptr<void>;

        class Connection;
        using ConnectionPtr = std::shared_ptr<Connection>;
        using ActiveCallBack = std::function<void(const ConnectionPtr &)>;

        struct BufferNode
        {
            BufferNode(void *buf, size_t len)
            :addr(buf), size(len)
            {

            }
            void *addr{nullptr};
            size_t size{0};
        };


        template<typename T>
        struct TimeOutEntry
        {   
            TimeOutEntry(const std::shared_ptr<T>& conn):
                conn_(conn)
            {

            }

            ~TimeOutEntry()
            {
                auto ptr = conn_.lock();
                if (ptr)
                {
                    ptr->OnTimeOut();
                }
            }

            std::weak_ptr<T> conn_;
        };


        class Connection : public Event
        {
            public:
                Connection(EventLoop *loop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr);
                virtual ~Connection() = default;

                void SetLocalAddr(const InetAddress &local);
                void SetPeerAddr(const InetAddress &peer);

                const InetAddress &LocalAddr() const;
                const InetAddress &PeerAddr() const;

                void SetContext(int type, const ContextPtr &context);
                void SetContext(int type, ContextPtr &&context);
                void ClearContext(int type);
                void ClearContext();
                
                template <typename T> std::shared_ptr<T> GetContext(int type) const
                {
                    auto iter = contexts_.find(type);
                    if (iter != contexts_.end())
                    {
                        /**
                         * dynamic_pointer_cast 是 C++11 引入的一个标准库函数，用于在智能指针（如 std::shared_ptr）之间进行动态类型转换。
                         * 它类似于 dynamic_cast，但适用于智能指针。
                         * 用法
                         * dynamic_pointer_cast 的主要作用是将一个基类类型的 std::shared_ptr 转换为派生类类型的 std::shared_ptr。
                         * 如果转换失败，返回的 std::shared_ptr 将为空。
                         */
                        return std::static_pointer_cast<T>(iter->second);
                    }
                    return std::shared_ptr<T>();
                }

                void SetActiveCallBack(const ActiveCallBack &cb);
                void SetActiveCallBack(ActiveCallBack &&cb);

                void Active();
                void Deactive();
                
                // 由不用的链接类型去关闭，tcp与udp 的关闭是不同的
                virtual void ForceClose() = 0;

            private:
                std::unordered_map<int, ContextPtr> contexts_;
                std::atomic<bool> active_{false};

                ActiveCallBack active_cb_;
                
            protected:
                InetAddress local_addr_;
                InetAddress peer_addr_;

        };
    }
}