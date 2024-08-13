/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-13 16:06:50
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-13 16:07:42
 * @FilePath: /VideoServer/src/mmedia/flv/FlvContext.h
 * @Description: 
 * 
 * Copyright (c) 2024 by duanran, All Rights Reserved. 
 */
#pragma once

#include "network/net/TcpConnection.h"
#include "mmedia/base/Packet.h"
#include "mmedia/base/MMediaCallBack.h"
#include <string>
#include <list>
#include <memory>

namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;

        class FlvContext
        {
        public:
            FlvContext(const TcpConnectionPtr &conn,MMediaCallBack *handler);
            ~FlvContext() = default;

            void  SendFlvHttpHeader(bool has_video, bool has_audio);
            void  WriteFlvHeader(bool has_video, bool has_audio);
            bool  BuildFlvFrame(PacketPtr &pkt, uint32_t timestamp);
            void Send();
            void WriteComplete(const TcpConnectionPtr &);
            bool Ready() const;

        private:
            char GetRtmpPacketType(PacketPtr &pkt);
            std::list<BufferNodePtr> bufs_;
            std::list<PacketPtr> out_packets_;
            TcpConnectionPtr connection_;
            uint32_t previous_size_{0};
            std::string http_header_;
            char out_buffer_[512];
            char *current_{nullptr};
            bool sending_{false};
            MMediaCallBack * handler_{nullptr};
        };
    }
}