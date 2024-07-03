/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 22:26:16
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-03 15:32:28
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpContext.h
 * @Description: Rtmp 消息解析, 作为一个上下文对象配合后续解析
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */

#pragma once
#include "mmedia/rtmp/RtmpHandshake.h"
#include "mmedia/rtmp/RtmpHeader.h"
#include "mmedia/base/Packet.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include <unordered_map>
#include <memory>

namespace vdse
{
    namespace mmedia
    {
        enum RtmpContextState
        {
            kRtmpHandshake = 1,
            kRtmpWaitDone,
            kRtmpMessage,
        };

        using namespace vdse::network;
        
        class RtmpContext
        {
            public:
                RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *hanlder, bool client = false);
                ~RtmpContext();

                int32_t Parse(MsgBuffer &buf);

                void OnwriteComplete();
                void StartHandshake();

                int32_t ParseMessage(MsgBuffer &buf);
                void MessageComplete(PacketPtr &&data);

            private:
                TcpConnectionPtr conneciton_;
                uint8_t state_{kRtmpHandshake};
                RtmpHandshake handshake_;
                RtmpHandler *rtmp_handler_{nullptr};
                // 上一个chunk stream id， csid， 的消息头 下同
                std::unordered_map<uint32_t, RtmpMsgHeaderPtr> in_message_headers_;
                // 保存与上一个时间戳的差值
                std::unordered_map<uint32_t, uint32_t> in_deltas_;
                std::unordered_map<uint32_t, PacketPtr> in_packet_;
                // 
                std::unordered_map<uint32_t, bool> in_ext_;
                int in_chunk_size_{128};
            
        };
        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    }
}
