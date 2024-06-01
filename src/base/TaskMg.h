#pragma once

#include "NoCopyable.h"
#include "Task.h"
#include <unordered_set>
#include <mutex>

namespace vdse
{
    namespace base
    {
        class TaskMg : public NoCopyable
        {
            public:
                TaskMg() = default;
                ~TaskMg() = default;

                void OnWork();
                void Add(TaskPtr &task);
                void Del(TaskPtr &task);
            private:
                std::unordered_set<TaskPtr> tasks_;
                std::mutex lock_;
        };
        #define sTm vdse::base::Singletion<vdse::base::TaskMg>::Instance()
    }
}