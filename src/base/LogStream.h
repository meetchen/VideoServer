/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-03 15:14:36
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 11:41:20
 * @FilePath: /VideoServer/src/base/LogStream.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */

#pragma once

#include "Logger.h"
#include <sstream>
#include <memory>


namespace vdse
{
    namespace base
    {   
        // extern Logger * g_logger;

        
        using LoggerPtr = std::shared_ptr<Logger>;

        extern LoggerPtr g_logger;

        class LogStream
        {
            public:
            
                LogStream(LoggerPtr &loger, const char *file, int line, LogLevel l, const char *func = nullptr);
                
                /**
                 * 在析构的时候执行日志的写操作
                */
                ~LogStream();

                template<typename T> LogStream & operator<<(const T&value)
                {
                    stream_ << value;
                    return *this; 
                }
            private:
                std::stringstream stream_;
                LoggerPtr logger_{nullptr};
        };
    }
}

#define LOG_TRACE   \
    if (vdse::base::g_logger && vdse::base::g_logger->GetLogLevel() <= vdse::base::kTrace)    \
        vdse::base::LogStream(vdse::base::g_logger, __FILE__, __LINE__, vdse::base::kTrace, __func__)

#define LOG_DEBUG   \
    if (vdse::base::g_logger && vdse::base::g_logger->GetLogLevel() <= vdse::base::kTrace)    \
        vdse::base::LogStream(vdse::base::g_logger, __FILE__, __LINE__, vdse::base::kDebug, __func__)

#define LOG_INFO   \
    if (vdse::base::g_logger && vdse::base::g_logger->GetLogLevel() <= vdse::base::kTrace)    \
        vdse::base::LogStream(vdse::base::g_logger, __FILE__, __LINE__, vdse::base::kInfo)

#define LOG_WARN vdse::base::LogStream(vdse::base::g_logger, __FILE__, __LINE__, vdse::base::kWarn)

#define LOG_ERROR vdse::base::LogStream(vdse::base::g_logger, __FILE__, __LINE__, vdse::base::kError)