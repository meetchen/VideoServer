#pragma once

#include <cstdint>
#include <string>

namespace vdse 
{
    namespace base
    {
        class TTime
        {
            public:
                // 表示当前的UTC时间，单位是毫秒
                static int64_t NowMS();
                static int64_t Now();
                static int64_t Now(int &year, int &month, int &dau, int &hour, int &minute, int &second);
                static std::string ISOTime();
        };
    }
}