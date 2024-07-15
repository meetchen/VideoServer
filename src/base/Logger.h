/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-01 16:06:03
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-06-02 22:09:00
 * @FilePath: /VideoServer/src/base/Logger.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "NoCopyable.h"
#include "FileLog.h"

#include <string>

namespace vdse
{
    namespace base
    {
        enum LogLevel
        {
            kTrace,
            kDebug,
            kInfo,
            kWarn,
            kError,
            kMaxNumOfLogLevel,
        };

        class Logger : public NoCopyable
        {
            public:
                Logger(const FileLogPtr &ptr);
                ~Logger() = default;
                void SetLogLevel(const LogLevel &level);
                LogLevel GetLogLevel();
                void write(const std::string &str);
            private:
                LogLevel level_{kDebug};
                FileLogPtr log_;

        };
    }
}