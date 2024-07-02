/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpContext.h
 * @Description  :  Rtmp 消息解析, 作为一个上下文对象配合后续解析
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-02 23:14:45
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#pragma once
#include "mmedia/rtmp/RtmpHandshake.h"
#include "mmedia/rtmp/RtmpHeader.h"
#include "mmedia/base/Packet.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include <unordered_map>

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
                std::unordered_map<uint32_t, RtmpMsgHeaderPtr> in_message_headers_;
                std::unordered_map<uint32_t, uint32_t> in_deltas_;
                std::unordered_map<uint32_t, PacketPtr> in_message_headers_;
                std::unordered_map<uint32_t, bool> in_ext_;
            
        };
    }
}
