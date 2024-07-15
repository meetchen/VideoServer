
#pragma once

#include <string>
#include <memory>

namespace vdse
{
    /// @brief 
    namespace base
    {
        enum RotateType
        {
            kRotateNone,
            kRotateHour,
            kRotateMinute,
            kRotateDay
        };

        /// @brief 
        class FileLog
        {
            public:
                FileLog() = default;
                ~FileLog() = default;


                bool Open(const std::string &filePath);

                size_t WriteLog(const std::string &msg);
                void Rotate(const std::string &file);
                void SetRotate(RotateType type);
    
                RotateType getRotateType();
                int64_t FileSize();
                std::string FilePath();
            
            private:
                RotateType rotate_{kRotateNone};
                int fd_{-1};
                std::string file_path_;
        };

        using FileLogPtr = std::shared_ptr<FileLog>;

    }
}