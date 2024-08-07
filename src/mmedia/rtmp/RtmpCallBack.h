/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 19:21:09
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-03 17:39:24
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpCallBack.h
 * @Description: 继承MmediaHandler, 为rtmp提供更多的接口定义
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "mmedia/base/MMediaCallBack.h"


namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        class RtmpCallBack : public MMediaCallBack
        {
            public:
                virtual bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param){ return false;}
                virtual bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param){ return false;}
                virtual void OnPause(const TcpConnectionPtr &conn, bool pause){}
                virtual void OnSeek(const TcpConnectionPtr &conn, double time){}
                virtual void OnPublishPrepare(const TcpConnectionPtr &conn){}

        };
    }
}

