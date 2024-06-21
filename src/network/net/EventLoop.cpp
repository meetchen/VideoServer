#include "EventLoop.h"
#include "network/base/Network.h"

using namespace vdms::network;

EventLoop::EventLoop():
epoll_fd_(::epoll_create(1024)),
epoll_events_(1024)
{

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
        memset(&epoll_events_[0], 0x3f, sizeof(struct epoll_event) * epoll_events_.size());
        auto ret = ::epoll_wait(epoll_fd_, (struct epoll_event *)&epoll_events_[0], epoll_events_.size(), -1);
        if (ret > 0)
        {
            for (int i = 0; i < ret; i++)
            {
                auto &ev = epoll_events_[i];
                // 检测是否发生事件错误
                if (ev.events & EPOLLERR)
                {

                }
                // 挂起事件检测，文件描述符被挂起、链接被关闭
                else if ((ev.events & EPOLLHUP) && !(ev.events & EPOLLIN))
                {

                }
                // 可读事件
                else if (ev.events & (EPOLLIN | EPOLLPRI))
                {

                }
                // 可写事件
                else if (ev.events & EPOLLOUT)
                {

                }
            }
            if (ret == epoll_events_.size ())
            {
                epoll_events_.resize(epoll_events_.size () * 2);
            }
        }
        else if (ret == 0)
        {

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