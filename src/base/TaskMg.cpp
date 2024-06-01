#include "TaskMg.h"
#include "TTime.h"


using namespace vdse::base;


void TaskMg::OnWork()
{
    std::lock_guard<std::mutex> lk(lock_);
    ino64_t now = TTime::NowMS();

    for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
    {
        if ((*it)->When() < now) 
        {
            (*it)->Run();
            // 如果回调函数没有重置下次执行时间，即单次任务
            if ((*it)->When() < now) 
            {
                it = tasks_.erase(it);
                continue;
            }
        }
    }
}

void TaskMg::Add(TaskPtr &task)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = tasks_.find(task);
    if (iter == tasks_.end())
    {
        tasks_.emplace(task);
    }
}

void TaskMg::Del(TaskPtr &task)
{
    std::lock_guard<std::mutex> lk(lock_);
    tasks_.erase(task);
}