/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-04 13:46:39
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-13 19:44:36
 * @FilePath: /VideoServer/src/mmedia/http/HttpCallBack.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */

#pragma once

#include "mmedia/base/MMediaCallBack.h"


namespace vdse  
{
    namespace mmedia
    {
        class HttpRequest;
        using HttpRequestPtr = std::shared_ptr<HttpRequest>;
        class HttpCallBack : virtual public vdse::mmedia::MMediaCallBack
        {
            public:
                virtual void OnSent(const TcpConnectionPtr &conn) = 0;
                virtual bool OnSentNextChunk(const TcpConnectionPtr &conn) = 0;   
                virtual void OnRequest(const TcpConnectionPtr &conn,const HttpRequestPtr &req,const PacketPtr &packet) = 0; 

        };
    }        

}