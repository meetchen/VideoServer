#pragma once

#include "NoCopyable.h"
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
                Logger() = default;
                ~Logger() = default;
                void SetLogLevel(const LogLevel &level);
                LogLevel GetLogLevel();
                void write(const std::string &str);
            private:
                LogLevel level_{kDebug};

        };
    }
}