#pragma once

#include "base/NoCopyable.h"
#include <future>
#include <memory>
#include <thread>
#include <condition_variable>
#include "EventLoop.h"
#include "network/net/TimingWheel.h"


namespace vdse
{
    namespace network
    {
        class EventLoopThread : public base::NoCopyable
        {
            public:
                EventLoopThread();
                ~EventLoopThread();
                void Run();
                std::thread& Thread();
                EventLoop *Loop() const;
            private:
                void StartEventLoop();
                EventLoop *loop_{nullptr};
                bool running_{false};
                std::mutex lock_;
                std::thread thread_;
                std::condition_variable condition_;
                std::once_flag once_;
                std::promise<int> promise_loop_;
        };
    }
}
