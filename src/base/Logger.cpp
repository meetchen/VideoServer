/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-03 15:14:36
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-06-03 16:24:22
 * @FilePath: /VideoServer/src/base/Logger.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "Logger.h"
#include <iostream>

using namespace vdse::base;


Logger::Logger(const FileLogPtr &ptr)
:log_(ptr)
{
    
}

void Logger::SetLogLevel(const LogLevel& level)
{
    level_ = level;
}

LogLevel Logger::GetLogLevel()
{
    return level_;
}

void Logger::write(const std::string &msg)
{
    if (log_)
    {
        log_->WriteLog(msg);
    }
    else
    {
        std::cout << msg;
    }
}