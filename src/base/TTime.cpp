#include <sys/time.h>
#include "TTime.h"

using namespace vdse::base;

int64_t TTime::NowMS()
{
    struct timeval tv;    
    // 获得当前精确时间（1970年1月1日到现在的时间
    gettimeofday(&tv, nullptr);
    return tv.tv_sec*1000+tv.tv_usec/1000;
}

int64_t TTime::Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec;
}


int64_t TTime::Now(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
    struct tm tm;
    time_t t = time(nullptr);
    // 转换为本地时间
    localtime_r(&t, &tm);

    year = tm.tm_year + 1900;
    month = tm.tm_mon + 1;
    day = tm.tm_mday;
    hour = tm.tm_hour;
    minute = tm.tm_min;
    second = tm.tm_sec;
    return t;
}
 

std::string TTime::ISOTime()
{
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, nullptr);
    time_t t = time(nullptr);
    localtime_r(&t, &tm);

    char buf[128] = {0};

    auto n = sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec
    );

    return std::string(buf, buf + n);

}