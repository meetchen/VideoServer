#pragma once

#include <string>
#include <sys/epoll.h>
#include <memory>

namespace vdse
{
    namespace network
    {
        class EventLoop;
        const int kEventRead = (EPOLLIN | EPOLLET | EPOLLPRI);
        const int kEventWrite = (EPOLLOUT | EPOLLET );
        
        class Event : public std::enable_shared_from_this<Event>
        {
            public:
                Event();
                Event(EventLoop *event_loop, int fd);
                ~Event();
                virtual void OnRead() {}
                virtual void OnWrite() {}
                virtual void OnError(const std::string &err_msg) {}
                virtual void OnClose() {}

                bool EnableWriting(bool enable);
                bool EnableReading(bool enable);
                int Fd() const;
            
            private:
                int fd_{-1};
                int event_{-1};
                EventLoop *loop_{nullptr};

        };
    }
}