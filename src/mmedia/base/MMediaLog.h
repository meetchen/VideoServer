/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 19:03:58
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 14:45:04
 * @FilePath: /VideoServer/src/mmedia/base/MMediaLog.h
 * @Description: 使用base定义的日志模块
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once 
#include "base/LogStream.h"
#include <iostream>


#define RTMP_DEBUG_ON 1

#ifdef RTMP_DEBUG_ON
#define RTMP_TRACE std::cout << "\nRTMP:"
#define RTMP_DEBUG LOG_DEBUG << "RTMP:"
#define RTMP_INFO LOG_INFO << "RTMP:"
#elif
#define RTMP_TRACE if(0) LOG_TRACE
#define RTMP_DEBUG if(0) LOG_DEBUG
#define RTMP_INFO if(0) LOG_INFO
#endif

#define RTMP_WARN LOG_WARN
#define RTMP_ERROR LOG_ERROR