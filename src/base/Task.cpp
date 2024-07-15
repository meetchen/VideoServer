#include "Task.h"
#include "TTime.h"


using namespace vdse::base;

Task::Task(const TaskCallBack &cb, int64_t interval)
:interval_(interval), when_(TTime::NowMS() + interval), cb_(cb)
{

}

Task::Task(const TaskCallBack &&cb, int64_t interval)
:interval_(interval), when_(TTime::NowMS() + interval), cb_(std::move(cb))
{

}

void Task::Run()
{
    if (cb_)
    {
        cb_(shared_from_this());
    }
}

void Task::Restart()
{
    when_ = interval_ + TTime::NowMS();
}