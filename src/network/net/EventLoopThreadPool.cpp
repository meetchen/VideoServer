#include "EventLoopThreadPool.h"
#include <sched.h>

using namespace vdse::network;

namespace
{
    void bind_cpu(std::thread &t, int n)
    {
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        // 将核心n 添加到cpu集合中
        CPU_SET(n, &cpu_set);
        // 设置亲和性 获取线程的底层句柄
        pthread_setaffinity_np(t.native_handle(),sizeof(cpu_set), &cpu_set);

    }
}


EventLoopThreadPool::EventLoopThreadPool(size_t threadNum, int start, int cpus)
{
    if (threadNum < 1) 
    {
        threadNum = 1;
    }

    for (int i = 0; i < threadNum; i++)
    {
        threads_.emplace_back(std::make_shared<EventLoopThread>());
        if (cpus > 0)
        {
            int n = (start + i) % cpus;
            bind_cpu(threads_.back()->Thread(), n);
        }
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

std::vector<EventLoop *> EventLoopThreadPool::GetLoops() const
{
    std::vector<EventLoop *> res;
    for (auto &t : threads_)
    {
        res.emplace_back(t->Loop());
    }
    return res;
}

EventLoop * EventLoopThreadPool::GetNextLoop()
{
    int index = loop_index_;
    loop_index_++;

    return threads_[index % this -> Size()] ->Loop();
}

size_t EventLoopThreadPool::Size()
{
    return threads_.size();
}

void EventLoopThreadPool::Start()
{
    for (auto &t : threads_)
    {
        t -> Run();
    }
}