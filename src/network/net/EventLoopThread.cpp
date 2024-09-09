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
    // 主线程调用
    std::call_once(once_, [this](){
        {
            std::lock_guard<std::mutex> lk(lock_);
            this -> running_ = true;
            this -> condition_.notify_all();
        }
        // 主线程阻塞获取返回值，可以用于同步，确保StartEventLoop启动顺利
        auto f = this->promise_loop_.get_future();
        f.get();
    });
}

void EventLoopThread::StartEventLoop()
{
    EventLoop loop;

    // 配合条件变量使用
    std::unique_lock<std::mutex> lk(lock_);
    condition_.wait(lk, [this](){
        return this->running_;
        });

    loop_ = &loop;
    this -> promise_loop_.set_value(1);
    loop.Loop();
    loop_ = nullptr;
}

EventLoop *EventLoopThread::Loop() const
{
    return loop_;
}

std::thread& EventLoopThread::Thread()
{
    return thread_;
}