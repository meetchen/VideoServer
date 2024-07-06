/**
 * @FilePath     : /VideoServer/src/network/net/EventLoop.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditors  : Please set LastEditors
 * @LastEditTime : 2024-07-06 21:58:40
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "EventLoop.h"
#include "network/base/Network.h"
#include "Event.h"
#include "base/TTime.h"
#include <sys/types.h>
#include <sys/socket.h>

using namespace vdse::network;

static thread_local EventLoop *t_local_eventLoop = nullptr;

EventLoop::EventLoop() : epoll_fd_(::epoll_create(1024)),
                         epoll_events_(1024)
{
    if (t_local_eventLoop)
    {
        NETWORK_ERROR << "there already had a eventloop!!!";
        exit(-1);
    }
    t_local_eventLoop = this;
}

EventLoop::~EventLoop()
{
    Quit();
}

void EventLoop::Loop()
{
    looping_ = true;
    int timeout = 1000;
    while (looping_)
    {
        memset(&epoll_events_[0], 0x00, sizeof(struct epoll_event) * epoll_events_.size());
        auto ret = ::epoll_wait(epoll_fd_, (struct epoll_event *)&epoll_events_[0], static_cast<int>(epoll_events_.size()), timeout);
        if (ret >= 0)
        {
            for (int i = 0; i < ret; i++)
            {
                auto &ev = epoll_events_[i];

                if (ev.data.fd <= 0)
                {
                    continue;
                }

                auto iter = events_.find(ev.data.fd);
                if (iter == events_.end())
                {
                    continue;
                }

                const EventPtr &event_ptr = iter->second;

                // 检测是否发生事件错误
                if (ev.events & EPOLLERR)
                {
                    int error = 0;
                    socklen_t len = sizeof(error);
                    getsockopt(event_ptr->Fd(), SOL_SOCKET, SO_ERROR, &error, &len);
                    event_ptr->OnError(strerror(error));
                }
                // 挂起事件检测，文件描述符被挂起、链接被关闭
                else if ((ev.events & EPOLLHUP) && !(ev.events & EPOLLIN))
                {
                    event_ptr->OnClose();
                }
                // 可读事件
                else if (ev.events & (EPOLLIN | EPOLLPRI))
                {
                    event_ptr->OnRead();
                }
                // 可写事件
                else if (ev.events & EPOLLOUT)
                {
                    event_ptr->OnWrite();
                }
            }
            if (ret == epoll_events_.size())
            {
                epoll_events_.resize(epoll_events_.size() * 2);
            }

            // 执行eventloop绑定的事件
            RunFunctions();
            
            // 更新时间轮
            auto now = base::TTime::NowMS();
            timing_wheel_.OnTimer(now);
        }
        else
        {
            NETWORK_ERROR << "epoll wait error. errno : " << errno;
        }
    }
}

void EventLoop::Quit()
{
    looping_ = false;
}

void EventLoop::AddEvent(const EventPtr &event)
{

    auto iter = events_.find(event->fd_);
    if (iter != events_.end())
    {
        return;
    }
    event->event_ |= kEventRead;
    events_[event->fd_] = event;

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event->fd_, &ev);
}

void EventLoop::DelEvent(const EventPtr &event)
{
    auto iter = events_.find(event->fd_);
    if (iter == events_.end())
    {
        return;
    }
    events_.erase(iter);

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->Fd();
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event->fd_, &ev);
}

bool EventLoop::EnableEventWriting(const EventPtr &event, bool enable)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        NETWORK_ERROR << "can't find event fd: " << event->Fd();
        return false;
    }
    if (enable)
    {
        event->event_ |= kEventWrite;
    }
    else
    {
        event->event_ &= ~kEventWrite;
    }
    events_[event->Fd()] = event;

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->Fd();
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->Fd(), &ev);
    return true;
}

bool EventLoop::EnableEventReading(const EventPtr &event, bool enable)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        NETWORK_ERROR << "can't find event fd: " << event->Fd();
        return false;
    }
    if (enable)
    {
        event->event_ |= kEventRead;
    }
    else
    {
        event->event_ |= ~kEventRead;
    }
    events_[event->Fd()] = event;

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->event_;
    ev.data.fd = event->Fd();
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->Fd(), &ev);
    return true;
}

void EventLoop::RunInLoop(const Func &f)
{
    if (InLoop())
    {
        f();
        return;
    }
    std::lock_guard<std::mutex> lk(lock_);
    functions_.emplace(f);
    WakeUp();
}

void EventLoop::RunInLoop(Func &&f)
{
    if (InLoop())
    {
        f();
        return;
    }
    std::lock_guard<std::mutex> lk(lock_);
    // 提示编译器 使用右值函数
    functions_.emplace(std::move(f));

    WakeUp();
}

void EventLoop::RunFunctions()
{
    std::lock_guard<std::mutex> lk(lock_);
    while (!functions_.empty())
    {
        auto f = functions_.front();
        f();
        functions_.pop();
    }
}

bool EventLoop::InLoop()
{
    return this == t_local_eventLoop;
}

void EventLoop::WakeUp()
{
    if (!pipe_event_)
    {
        pipe_event_ = std::make_shared<PipeEvent>(this);
        AddEvent(pipe_event_);
    }
    int msg = 1;
    pipe_event_->Write(reinterpret_cast<const char *>(&msg), sizeof(msg));
}

void EventLoop::AssertLoopInThread()
{
    if (!InLoop())
    {
        NETWORK_ERROR << "Forbidden in this thread ";
        exit(-1);
    }
}


void EventLoop::RunAfter(double delay, const Func &cb)
{
    if (InLoop())
    {
        timing_wheel_.RunAfter(delay, cb);
    }
    else
    {
        RunInLoop([this, delay, cb](){
            timing_wheel_.RunAfter(delay, cb);
        });
    }
}

void EventLoop::RunAfter(double delay, Func &&cb)
{
    if (InLoop())
    {
        timing_wheel_.RunAfter(delay, std::move(cb));

    }
    else
    {
        RunInLoop([this, delay, cb](){
            timing_wheel_.RunAfter(delay, std::move(cb));
        });
    }
}

void EventLoop::RunEvery(double interval, const Func &cb)
{
    if (InLoop())
    {
        timing_wheel_.RunEvery(interval, cb);
    }
    else
    {
        RunInLoop([this, interval, cb](){
            timing_wheel_.RunEvery(interval, cb);
        });
    }
}

void EventLoop::RunEvery(double interval, Func &&cb)
{
    if (InLoop())
    {
        timing_wheel_.RunEvery(interval, std::move(cb));
    }
    else
    {
        RunInLoop([this, interval, cb](){
            timing_wheel_.RunEvery(interval, std::move(cb));
        });
    }
}

void EventLoop::InsertEntry(uint32_t delay, const EntryPtr& entry)
{
    timing_wheel_.InsertEntry(delay, entry);
}

void EventLoop::InsertEntry(uint32_t delay, EntryPtr&& entry)
{
    timing_wheel_.InsertEntry(delay, std::move(entry));
}