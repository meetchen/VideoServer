/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-09 11:36:49
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 14:17:47
 * @FilePath: /VideoServer/src/base/AppInfo.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "json/json.h"
#include <string>
#include <memory>

namespace vdse
{
    namespace base
    {
        using std::string;

        class DomainInfo;
        class AppInfo
        {
        public:
            /**
             * @description: app 禁止隐式类型转换
             * @param {DomainInfo} &d
             * @return {*}
             */            
            explicit AppInfo(DomainInfo &d);

            bool ParseAppInfo(Json::Value &root);

            DomainInfo &domain_info;
            std::string domain_name;
            std::string app_name;
            uint32_t max_buffer{1000}; // 最多缓存多少帧
            bool rtmp_support{false}; // 是否支持RTMP 下同
            bool flv_support{false};
            bool hls_support{false};
            uint32_t content_latency{3*1000}; // 内容延迟 延迟多少发
            uint32_t stream_idle_time{30*1000}; // 当一个流空闲 没有被使用 多久进行关闭
            uint32_t stream_timeout_time{30*1000}; // 当一个流多久没有反馈，有使用，超时关闭
        };
    }
}