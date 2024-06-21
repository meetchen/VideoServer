#pragma once

#include <vector>
#include <cstring>
#include <sys/epoll.h>

namespace vdms
{
    namespace network
    {
        class EventLoop
        {
            public:
                EventLoop();
                ~EventLoop();
            
                void Loop();
                void Quit();
            
            private:
                bool looping_{false};
                int epoll_fd_{-1};
                std::vector<struct epoll_event> epoll_events_;
        };

    }
}