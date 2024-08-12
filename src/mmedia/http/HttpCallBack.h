/**
 * @FilePath     : /VideoServer/src/mmedia/http/HttpCallBack.h
 * @Description  :  
 * @Author       : duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-08-04 13:49:15
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#pragma once

#include "mmedia/base/MMediaCallBack.h"


namespace vdse  
{
    namespace mmedia
    {
        class HttpRequest;
        using HttpRequestPtr = std::shared_ptr<HttpRequest>;
        class HttpCallBack : public vdse::mmedia::MMediaCallBack
        {
            public:
                virtual void OnSent(const TcpConnectionPtr &conn) = 0;
                virtual bool OnSentNextChunk(const TcpConnectionPtr &conn) = 0;   
                virtual void OnRequest(const TcpConnectionPtr &conn,const HttpRequestPtr &req,const PacketPtr &packet) = 0; 

        };
    }        

}