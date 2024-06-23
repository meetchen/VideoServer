#include "EventLoop.h"
#include "network/base/Network.h"
#include "Event.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

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
    while (looping_)
    {
        memset(&epoll_events_[0], 0x00, sizeof(struct epoll_event) * epoll_events_.size());
        auto ret = ::epoll_wait(epoll_fd_, (struct epoll_event *)&epoll_events_[0], static_cast<int>(epoll_events_.size()), -1);
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
        event->event_ |= ~kEventWrite;
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
