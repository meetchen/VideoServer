#include "EventLoopThread.h"

using namespace vdse::network;


EventLoopThread::EventLoopThread()
:thread_([this](){this->StartEventLoop();})
{

}

EventLoopThread::~EventLoopThread()
{
    Run();
    if (loop_)
    {
        loop_->Quit();
    }

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void EventLoopThread::Run()
{
    std::call_once(once_, [this](){
        {
            std::lock_guard<std::mutex> lk(lock_);
            this -> running_ = true;
            this -> condition_.notify_all();
        }
        // 阻塞获取返回值，可以用于同步
        auto f = this->promise_loop_.get_future();
        f.get();
    });
}

void EventLoopThread::StartEventLoop()
{
    EventLoop loop;
    std::unique_lock<std::mutex> lk(lock_);
    condition_.wait(lk, [this](){return this->running_;});
    loop_ = &loop;
    this -> promise_loop_.set_value(1);
    loop.Loop();
    loop_ = nullptr;
}

EventLoop *EventLoopThread::Loop() const
{
    return loop_;
}