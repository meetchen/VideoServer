
#include "Config.h"
#include "LogStream.h"

using namespace vdse::base;

bool Config::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "config file : " << file;
    
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(file, root))
    {
        LOG_DEBUG << "config file : " << file << " parse error .";
        return false;
    }

    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        name_ = nameObj.asString();
    }

    Json::Value cpuStartObj = root["cpu_start"];
    if (!cpuStartObj.isNull())
    {
        cpu_start_ = cpuStartObj.asInt();
    }

    Json::Value threadsObj = root["threads"];
    if (!threadsObj.isNull())
    {
        thread_nums_ = threadsObj.asInt();
    }

    Json::Value logObj = root["log"];
    if (!logObj.isNull())
    {
        ParseLogInfo(logObj);
    }

    return true;

}

bool Config::ParseLogInfo(const Json::Value &root)
{
    log_info_ = std::make_shared<LogInfo>();

    Json::Value levelObj = root["level"];
    if (!levelObj.isNull())
    {
        auto level =  levelObj.asString();
        if (level == "DEBUG") 
        {
            log_info_->level = kDebug;
        }
        else if (level == "INFO")
        {
            log_info_->level = kInfo;
        }
        else if (level == "WARN")
        {
            log_info_->level = kWarn;
        }
        else if (level == "ERROR")
        {
            log_info_->level = kError;
        }
        else if (level == "TRACE")
        {
            log_info_->level = kTrace;
        }
    }

    Json::Value pathObj = root["path"];
    if (!pathObj.isNull())
    {
        log_info_->path = pathObj.asString();
    }

    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        log_info_->name = nameObj.asString();
    }

    Json::Value rotateObj = root["rotate"];
    if (!rotateObj.isNull())
    {
        auto rotate = rotateObj.asString();
        if (rotate == "HOUR")
        {
            log_info_ -> rotate = kRotateHour;
        }
        else if (rotate == "DAY")
        {
            log_info_ -> rotate = kRotateDay;
        }
    }
    return true;
}

LogInfoPtr& Config::GetLogInfo()
{
    return log_info_;
}


bool ConfigMg::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "load config file : " << file;
    ConfigPtr config = std::make_shared<Config>();
    if (config->LoadConfig(file))
    {
        std::lock_guard<std::mutex> lk(lock_);
        config_ = config;
        return false;
    }
    return false;
}

ConfigPtr ConfigMg::GetConfigPtr()
{
    std::lock_guard<std::mutex> lk(lock_);
    return config_;
}