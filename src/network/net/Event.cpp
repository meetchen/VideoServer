#include "Event.h"
#include "EventLoop.h"
#include <unistd.h>
#include <iostream>

using namespace vdse::network;

Event::Event(EventLoop *event_loop)
:loop_(event_loop)
{
    
}

Event::Event(EventLoop *event_loop, int fd)
:loop_(event_loop),
fd_(fd)
{

}

Event::~Event()
{
    if (fd_ > 0)
    {
        ::close(fd_);
        fd_ = -1;
    }
}

bool Event::EnableWriting(bool enable)
{
    return loop_ -> EnableEventWriting(shared_from_this(), enable);
}

bool Event::EnableReading(bool enable)
{
    return loop_ -> EnableEventReading(shared_from_this(), enable);

}

int Event::Fd() const
{
    return fd_;
}