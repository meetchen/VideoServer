/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-09 11:36:50
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 11:40:33
 * @FilePath: /VideoServer/src/base/DomainInfo.cpp
 * @Description: 每个域名对应的类
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "base/DomainInfo.h"
#include "base/LogStream.h"
#include "json/json.h"
#include "base/AppInfo.h"
#include <fstream>


using namespace vdse::base;

const string& DomainInfo::DomainName() const
{
    return name_;
}
const string &DomainInfo::Type() const
{
    return type_;
}
bool DomainInfo::ParseDomainInfo(const std::string &file)
{
    LOG_DEBUG << "domain file:" << file;
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::ifstream in(file);
    std::string err;
    auto ok = Json::parseFromStream(reader,in,&root,&err);
    if(!ok)
    {
        LOG_ERROR << "domain config file:" << file << " parse error.err:" << err;
        return false;
    }

    Json::Value domainObj = root["domain"];
    if(domainObj.isNull())
    {
        LOG_ERROR << "domain config invalid cotent.no domain.";
        return false;
    }  

    Json::Value nameObj = domainObj["name"];
    if(!nameObj.isNull())
    {
        name_ = nameObj.asString();
    }
    Json::Value typeObj = domainObj["type"];
    if(!typeObj.isNull())
    {
        type_ = typeObj.asString();
    }

    Json::Value appsObj = domainObj["app"];
    if(appsObj.isNull())
    {
        LOG_ERROR << "domain config invalid cotent.no apps.";
        return false;
    }
    for(auto &aObj:appsObj)
    {
        AppInfoPtr appinfo = std::make_shared<AppInfo>(*this);
        auto ret = appinfo->ParseAppInfo(aObj);
        if(ret)
        {
            std::lock_guard<std::mutex> lk(lock_);
            appinfos_.emplace(appinfo->app_name,appinfo);
        }
    }
    return true;
}
AppInfoPtr DomainInfo::GetAppInfo(const string &app_name)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = appinfos_.find(app_name);
    if(iter != appinfos_.end())
    {
        return iter->second;
    }
    return AppInfoPtr();
}