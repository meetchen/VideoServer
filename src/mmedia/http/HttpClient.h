/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-13 12:32:40
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-13 12:32:50
 * @FilePath: /VideoServer/src/mmedia/http/HttpClient.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "network/TcpClient.h"
#include "network/net/EventLoop.h"
#include "network/base/InetAddress.h"
#include "mmedia/http/HttpCallBack.h"
#include "mmedia/http/HttpRequest.h"
#include <functional>
#include <memory>

namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        using TcpClientPtr = std::shared_ptr<TcpClient>;

        class HttpClient
        {
            public:
                HttpClient(EventLoop *loop,HttpCallBack *handler);
                ~HttpClient();

                void SetCloseCallback(const CloseConnectionCallBack &cb);
                void SetCloseCallback(CloseConnectionCallBack &&cb);

                void Get(const std::string &url);
                void Post(const std::string &url,const PacketPtr &packet);

            private:  
                void OnWriteComplete(const TcpConnectionPtr &conn);
                void OnConnection(const TcpConnectionPtr& conn,bool connected);
                void OnMessage(const TcpConnectionPtr& conn,MsgBuffer &buf);        
                bool ParseUrl(const std::string &url);
                void CreateTcpClient();  
                EventLoop *loop_{nullptr};
                InetAddress addr_;
                HttpCallBack *handler_{nullptr};
                TcpClientPtr tcp_client_;
                std::string url_;
                bool is_post_{false};
                CloseConnectionCallBack close_cb_;
                HttpRequestPtr request_;
                PacketPtr out_packet_;
        };
    }
}