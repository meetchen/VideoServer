/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 19:18:50
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-02 20:03:50
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpServer.h
 * @Description: Rtmp的服务端，继承TcpServer，组合RtmpHandler对上层进行回调
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "network/TcpServer.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include "mmedia/rtmp/RtmpHandshake.h"


namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        class RtmpServer : public TcpServer
        {
            public:
                RtmpServer(EventLoop *loop, InetAddress &local, RtmpHandler *handler = nullptr);
                ~RtmpServer();

                void Start() override;
                void Stop() override;
            private:
                void OnNewConneciton(const TcpConnectionPtr &conn);
                void OnDestroyed(const TcpConnectionPtr &conn);
                void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf);
                void OnWriteComplete(const ConnectionPtr &conn);
                void OnActive(const ConnectionPtr &conn);
                
                RtmpHandler *rtmp_handler_{nullptr};

        };
    }
}
