/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-03 15:17:38
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 14:41:34
 * @FilePath: /VideoServer/src/base/Config.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <memory>
#include <string>
#include <mutex>
#include "base/Singleton.h"
#include "base/NoCopyable.h"
#include "json/json.h"
#include "base/Logger.h"
#include "base/FileMg.h"
#include <unordered_map>

namespace vdse
{
    namespace base
    {
        using std::string;
        struct LogInfo
        {
            RotateType rotate{kRotateNone};
            std::string name;
            std::string path;
            LogLevel level;
        };

        using LogInfoPtr = std::shared_ptr<LogInfo>;

        struct ServiceInfo
        {
            string addr;
            uint16_t port;
            string protocol;
            string transport;
        };
        using ServiceInfoPtr = std::shared_ptr<ServiceInfo>;

        class DomainInfo;
        class AppInfo;
        using DomainInfoPtr = std::shared_ptr<DomainInfo>;
        using AppInfoPtr = std::shared_ptr<AppInfo>;

        class Config
        {
            public:
                Config() = default;
                ~Config() = default;
                bool LoadConfig(const std::string &file);
                LogInfoPtr& GetLogInfo();
                
                const std::vector<ServiceInfoPtr> & GetServiceInfos();
                const ServiceInfoPtr &GetServiceInfo(const string &protocol,const string &transport);
                bool ParseServiceInfo(const Json::Value &serviceObj);

                AppInfoPtr GetAppInfo(const string &domain,const string &app);
                DomainInfoPtr GetDomainInfo(const string &domain);
                
            private:
                bool ParseLogInfo(const Json::Value &root);

                bool ParseDirectory(const Json::Value &root);
                bool ParseDomainPath(const string &path);
                bool ParseDomainFile(const string &file);

                LogInfoPtr log_info_;
                std::string name_;
                int32_t cpu_start_;
                int32_t thread_nums_;
                std::mutex lock_;


                std::vector<ServiceInfoPtr> services_;
                std::unordered_map<std::string,DomainInfoPtr> domaininfos_;
        };
        
        using ConfigPtr = std::shared_ptr<Config>;

        class ConfigMg
        {
            public:
                ConfigMg() = default;
                ~ConfigMg() = default;
                bool LoadConfig(const std::string &file);
                ConfigPtr GetConfig();

                LogInfoPtr& GetLogInfo();


            private:
                std::mutex lock_;
                ConfigPtr config_;

        };

        #define sConfigMg vdse::base::Singletion<vdse::base::ConfigMg>::Instance()
    }
}