#include "PipeEvent.h"
#include <unistd.h>
#include "network/base/Network.h"
#include <iostream>
#include <fcntl.h>


using namespace vdse::network;


PipeEvent::PipeEvent(EventLoop *loop)
:Event(loop)
{
    int fd[2] = {0};
    int ret = ::pipe2(fd, O_NONBLOCK);

    if (ret < 0)
    {
        NETWORK_ERROR << "pipe2 init error";
        exit(-1);
    }
    fd_ = fd[0];
    write_fd_ = fd[1];

}

PipeEvent::~PipeEvent()
{
    if (write_fd_ > 0)
    {
        ::close(write_fd_);
        write_fd_ = -1;
    }
}

void PipeEvent::OnRead() 
{
    int temp;
    auto ret = ::read(fd_, &temp, sizeof(temp));
    if (ret < 0)
    {
        NETWORK_ERROR << "pipe read error, error : " << errno;
        return;
    }
    std::cout << "pipe read temp : " << temp << std::endl;
}

void PipeEvent::OnError(const std::string &err_msg) 
{
    NETWORK_ERROR << "Pipe2 Error : "  << err_msg;
}

void PipeEvent::Write(const char* data, size_t len)
{
    int ret = ::write(write_fd_, data, len);
    if (ret < 0)
    {
        std::cout << "write error, errno :" << errno << std::endl;
    }


}

void PipeEvent::OnClose() 
{
    if (write_fd_ > 0)
    {
        ::close(write_fd_);
        write_fd_ = -1;
    }
}
