/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpClient.h
 * @Description  :  RtmpClient 推拉流
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-07 13:28:00
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#pragma once

#include "network/net/EventLoop.h"
#include "network/net/TcpConnection.h"
#include "network/TcpClient.h"
#include "network/base/InetAddress.h"
#include "mmedia/rtmp/RtmpCallBack.h"
#include <functional>
#include <memory>
#include <string>


namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;

        class RtmpClient
        {
            public:
                RtmpClient(EventLoop *loop, RtmpCallBack* rtmp_callback);
                ~RtmpClient();

                void SetCloseCallback(const CloseConnectionCallBack &cb);
                void SetCloseCallback(CloseConnectionCallBack &&cb);

                void Play(const std::string &url);
                void Publish(const std::string &url);
                
                void Send(PacketPtr &&data);
            private:  
                void OnWriteComplete(const TcpConnectionPtr &conn);
                void OnConnection(const TcpConnectionPtr& conn,bool connected);
                void OnMessage(const TcpConnectionPtr& conn,MsgBuffer &buf);        
                bool ParseUrl(const std::string &url);
                void CreateTcpClient();  

                EventLoop *loop_{nullptr};
                InetAddress addr_;
                RtmpCallBack *rtmp_callback_{nullptr};
                TcpClientPtr tcp_client_;
                std::string url_;
                bool is_player_{false};
                CloseConnectionCallBack close_cb_;
        };
    }

}
