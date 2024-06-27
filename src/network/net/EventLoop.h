#pragma once

#include "network/net/PipeEvent.h"
#include <vector>
#include <cstring>
#include <sys/epoll.h>
#include <memory>
#include "network/net/Event.h"
#include <unordered_map>
#include <functional>
#include <mutex>
#include <queue>
#include "network/net/TimingWheel.h"


namespace vdse
{
    namespace network
    {
        using EventPtr = std::shared_ptr<Event>;
        using Func = std::function<void()>;
        class EventLoop
        {
            public:
                EventLoop();
                ~EventLoop();
            
                void Loop();
                void Quit();

                void AddEvent(const EventPtr& event);
                void DelEvent(const EventPtr& event);

                bool EnableEventWriting(const EventPtr& event, bool enable);
                bool EnableEventReading(const EventPtr& event, bool enable);

                void RunFunctions();
                void RunInLoop(const Func&f);           
                void RunInLoop(Func&& f);
                bool InLoop();

                void AssertLoopInThread();

                // 基于时间轮的  
                void RunAfter(double delay, const Func &cb);
                void RunAfter(double delay, Func &&cb);
                void RunEvery(double interval, const Func &cb);
                void RunEvery(double interval, Func &&cb);
                 
                void InsertEntry(uint32_t delay, const EntryPtr& entry);
                void InsertEntry(uint32_t delay, EntryPtr&& entry);

            private:
                bool looping_{false};
                int epoll_fd_{-1};
                void WakeUp();
                std::vector<struct epoll_event> epoll_events_;
                std::unordered_map<int, EventPtr> events_;
                std::queue<Func> functions_;
                std::mutex lock_;
                PipeEventPtr pipe_event_;
                // 时间轮 定时器
                TimingWheel timing_wheel_;
        };

    }
}