#pragma once

#include "Logger.h"
#include <sstream>

namespace vdse
{
    namespace base
    {   
        extern Logger * g_logger;
        class LogStream
        {
            public:
                LogStream(Logger *loger, const char *file, int line, LogLevel l, const char *func = nullptr);
                ~LogStream();
                template<typename T> LogStream & operator<<(const T&value)
                {
                    stream_ << value;
                    return *this; 
                }
            private:
                std::stringstream stream_;
                Logger* logger_{nullptr};
        };
    }
}

#define LOG_TRACE   \
    if (vdse::base::g_logger->GetLogLevel() <= vdse::base::kTrace)    \
        vdse::base::LogStream(vdse::base::g_logger, __FILE__, __linux__, vdse::base::kTrace, __func__)

#define LOG_DEBUG   \
    if (vdse::base::g_logger->GetLogLevel() <= vdse::base::kTrace)    \
        vdse::base::LogStream(vdse::base::g_logger, __FILE__, __linux__, vdse::base::kDebug, __func__)

#define LOG_INFO   \
    if (vdse::base::g_logger->GetLogLevel() <= vdse::base::kTrace)    \
        vdse::base::LogStream(vdse::base::g_logger, __FILE__, __linux__, vdse::base::kInfo)

#define LOG_WARN vdse::base::LogStream(vdse::base::g_logger, __FILE__, __linux__, vdse::base::kWarn)

#define LOG_ERROR vdse::base::LogStream(vdse::base::g_logger, __FILE__, __linux__, vdse::base::kError)