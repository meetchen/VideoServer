#pragma once

#include "EventLoop.h"
#include "base/NoCopyable.h"
#include "EventLoopThread.h"
#include <vector>
#include <atomic>

namespace vdse
{
    namespace network
    {   
        using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
        class EventLoopThreadPool : public base::NoCopyable
        {
            public:
                EventLoopThreadPool(size_t threadNum, int start = 0, int cpus = 4);
                ~EventLoopThreadPool();
                std::vector<EventLoop *> GetLoops() const;
                EventLoop * GetNextLoop();
                size_t Size();
                void Start();
            private:
                std::vector<EventLoopThreadPtr> threads_;
                std::atomic_int64_t loop_index_{0};
        };
    }
}