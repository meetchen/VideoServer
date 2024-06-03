#pragma once

#include <memory>
#include <string>
#include <mutex>
#include "Singleton.h"
#include "NoCopyable.h"
#include "json/json.h"
#include "Logger.h"
#include "FileMg.h"

namespace vdse
{
    namespace base
    {
        struct LogInfo
        {
            RotateType rotate{kRotateNone};
            std::string name;
            std::string path;
            LogLevel level;
        };

        using LogInfoPtr = std::shared_ptr<LogInfo>;

        class Config
        {
            public:
                Config() = default;
                ~Config() = default;
                bool LoadConfig(const std::string &file);
                LogInfoPtr& GetLogInfo();

                
            private:
                bool ParseLogInfo(const Json::Value &root);

                LogInfoPtr log_info_;
                std::string name_;
                int32_t cpu_start_;
                int32_t thread_nums_;

        };
        
        using ConfigPtr = std::shared_ptr<Config>;

        class ConfigMg
        {
            public:
                ConfigMg() = default;
                ~ConfigMg() = default;
                bool LoadConfig(const std::string &file);
                ConfigPtr GetConfig();
            private:
                std::mutex lock_;
                ConfigPtr config_;
        };

        #define sConfigMg vdse::base::Singletion<vdse::base::ConfigMg>::Instance()
    }
}