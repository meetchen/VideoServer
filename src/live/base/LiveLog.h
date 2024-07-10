/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 14:42:49
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 14:44:37
 * @FilePath: /VideoServer/src/live/base/LiveLog.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once 
#include "base/LogStream.h"
#include <iostream>


#define LIVE_DEBUG_ON 1

#ifdef LIVE_DEBUG_ON
#define LIVE_TRACE std::cout << "\nLIVE:"
#define LIVE_DEBUG LOG_DEBUG << "LIVE:"
#define LIVE_INFO LOG_INFO << "LIVE:"
#elif
#define LIVE_TRACE if(0) LOG_TRACE
#define LIVE_DEBUG if(0) LOG_DEBUG
#define LIVE_INFO if(0) LOG_INFO
#endif

#define LIVE_WARN LOG_WARN
#define LIVE_ERROR LOG_ERROR