#pragma once

#include "FileLog.h"
#include "NoCopyable.h"
#include "Singleton.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace vdse
{
    namespace base
    {
        class FileMg : public NoCopyable
        {
            public:
                FileMg() = default;
                ~FileMg() = default;
                void OnCheck();
                FileLogPtr GetFileLog(const std::string &file_name);
                void RemoveFileLog(const FileLogPtr &log);
                void RotateDays(const FileLogPtr &file);
                void RotateHours(const FileLogPtr &file);
                void RotateMinutes(const FileLogPtr &file);
            private:
                std::unordered_map<std::string, FileLogPtr> logs_;
                std::mutex lock_;
                int last_day_{-1};
                int last_hour_{-1};
                int last_year_{-1};
                int last_month_{-1};
                int last_minute_{-1};

        };
    }
}

#define sFileMg vdse::base::Singletion<vdse::base::FileMg>::Instance()