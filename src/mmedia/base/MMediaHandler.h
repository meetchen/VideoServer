/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 18:27:35
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-01 18:32:40
 * @FilePath: /VideoServer/src/mmedia/base/MMediaHandler.h
 * @Description: 多媒体模块的各种协议的回调 抽象基类 用于各种协议的继承， 类似于Java的接口
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "base/NoCopyable.h"
#include "network/net/TcpConnection.h"
#include "Packet.h"
#include <memory>
namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        class MMediaHandler: public vdse::base::NoCopyable
        {
            public:
                virtual void OnNewConnection(const TcpConnectionPtr &conn) = 0;
                virtual void OnConnectionDestroy(const TcpConnectionPtr &conn) = 0;
                virtual void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) = 0;
                virtual void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) = 0;
                virtual void OnActive(const ConnectionPtr &conn) = 0;
        };
    }
}