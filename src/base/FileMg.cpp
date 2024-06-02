#include "FileMg.h"
#include "TTime.h"
#include "StringUtils.h"
#include <sstream>

using namespace vdse::base;

// 匿名命令空间，只在当前的文件中有效
namespace
{
    static vdse::base::FileLogPtr file_log_nullptr;
}

void FileMg::Oncheck()
{
    bool day_change = false, hour_change = false, minute_change = false;

    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    TTime::Now(year, month, day, hour, minute, second);

    if (last_day_ == -1)
    {
        last_day_ = day;
        last_hour_ = hour;
        last_minute_ = minute;
    }

    if (last_day_ != day)
    {
        day_change = true;
    }

    if (last_hour_ != hour)
    {
        hour_change = true;
    }

    if (last_minute_ != minute)
    {
        minute_change = true;
    }


    if (!last_day_ && !last_hour_)
    {
        return;
    }

    std::lock_guard<std::mutex> lk(lock_);

    for (auto &[path, logPtr] : logs_)
    {
        if (day_change && logPtr->getRotateType() == kRotateDay)
        {
            RotateDays(logPtr);
        }
        else if (hour_change && logPtr->getRotateType() == kRotateHour)
        {
            RotateHours(logPtr);
        } else if (minute_change && logPtr->getRotateType() == kRotateMinute)
        {
            RotateMinutes(logPtr);
        }
    }

    last_day_ = day;
    last_hour_ = hour;
    last_year_ = year;
    last_month_ = month;
    last_minute_ = minute;
}

FileLogPtr FileMg::GetFileLog(const std::string &file_name)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = logs_.find(file_name);
    if (iter != logs_.end())
    {
        return iter->second;
    }

    FileLogPtr log = std::make_shared<FileLog>();
    if (!log->Open(file_name))
    {
        return file_log_nullptr;
    }
    logs_.emplace(file_name, log);

    return log;
}

void FileMg::RemoveFileLog(const FileLogPtr &log)
{
    std::lock_guard<std::mutex> lk(lock_);
    logs_.erase(log->FilePath());
}

void FileMg::RotateDays(const FileLogPtr &file)
{
    if (file->FileSize() > 0)
    {
        char buf[128] = {0};
        sprintf(buf,"_%04d-%02d-%02d", last_year_, last_month_, last_day_);
        
        auto file_path = file->FilePath();
        auto path = StringUtils::FilePath(file_path);
        auto file_ext = StringUtils::Extension(file_path);
        auto file_name = StringUtils::FileName(file_path);

        std::stringstream ss;

        ss << path << file_name << buf << file_ext;

        file->Rotate(ss.str());
    }
}

void FileMg::RotateHours(const FileLogPtr &file)
{
     if (file->FileSize() > 0)
    {
        char buf[128] = {0};
        sprintf(buf,"_%04d-%02d-%02d-%02d", last_year_, last_month_, last_day_, last_hour_);
        
        auto file_path = file->FilePath();
        auto path = StringUtils::FilePath(file_path);
        auto file_ext = StringUtils::Extension(file_path);
        auto file_name = StringUtils::FileName(file_path);

        std::stringstream ss;

        ss << path << file_name << buf << file_ext;

        file->Rotate(ss.str());
    }
}

void FileMg::RotateMinutes(const FileLogPtr &file)
{
     if (file->FileSize() > 0)
    {
        char buf[128] = {0};
        sprintf(buf,"_%04d-%02d-%02d-%02d-%02d", last_year_, last_month_, last_day_, last_hour_, last_minute_);
        
        auto file_path = file->FilePath();
        auto path = StringUtils::FilePath(file_path);
        auto file_ext = StringUtils::Extension(file_path);
        auto file_name = StringUtils::FileName(file_path);

        std::stringstream ss;

        ss << path << file_name << buf << file_ext;

        file->Rotate(ss.str());
    }
}