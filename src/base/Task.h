/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-03 15:14:36
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-06-03 20:55:49
 * @FilePath: /VideoServer/src/base/Task.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace vdse
{
    namespace base
    {
        class Task;

        using TaskPtr = std::shared_ptr<Task>;
        using TaskCallBack = std::function<void(const TaskPtr &)>;

        class Task : public std::enable_shared_from_this<Task>
        {
        public:
            Task(const TaskCallBack &cb, int64_t interval);
            Task(const TaskCallBack &&cb, int64_t interval);
            void Run();
            void Restart();
            int64_t When() const
            {
                return when_;
            }

        private:
            int64_t interval_{0};
            int64_t when_{0};
            TaskCallBack cb_;
        };
    }
}