/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 22:26:16
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:16:36
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpContext.h
 * @Description: Rtmp 消息解析, 作为一个上下文对象配合后续解析
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */

#pragma once
#include "mmedia/rtmp/RtmpHandshake.h"
#include "mmedia/rtmp/RtmpHeader.h"
#include "mmedia/base/Packet.h"
#include "mmedia/rtmp/RtmpCallBack.h"
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

        enum RtmpEventType
        {
            kRtmpEventTypeStreamBegin = 0,
            kRtmpEventTypeStreamEOF,
            kRtmpEventTypeStreamDry,
            kRtmpEventTypeSetBufferLength,
            kRtmpEventTypeStreamsRecorded,
            kRtmpEventTypePingRequest ,
            kRtmpEventTypePingResponse,
        };

        using namespace vdse::network;

        const int MESSAGE_CHUNK_SIZE = 4096;
        
        class RtmpContext
        {
            public:
                RtmpContext(const TcpConnectionPtr &conn, RtmpCallBack *hanlder, bool client = false);
                
                ~RtmpContext();

                int32_t Parse(MsgBuffer &buf);

                void OnwriteComplete();
         
                void StartHandshake();

                int32_t ParseMessage(MsgBuffer &buf);

                void MessageComplete(PacketPtr &&data);

                void HandleAMFMessage(PacketPtr &packet, bool);


                
                bool BuildChunk(const PacketPtr &packet, uint32_t timestamp = 0, bool fmt0 = false);

                void Send();
                
                bool Ready() const;

            private:

                bool BuildChunk(PacketPtr &&packet, uint32_t timestamp = 0, bool fmt0 = false);

                // ----发送协议控制消息统一接口----

                void SendSetChunkSize();
                
                void SendAckWindowSize();

                void SendSetPeerBandwidth();

                /**
                 * @brief        :  确认已经收到的字节数
                 * @return        {*}
                **/                
                void SentBytesRecv();

                void SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2);


                // -----接受协议控制消息统一接口-----

                void HandleChunkSize(PacketPtr& packet);

                void HandleAckWindowSize(PacketPtr &packet);

                void HandleUserMessage(PacketPtr &packet);


                // 在发送完一次消息后，整理缓冲区，检查是否还有遗留
                void CheckAfterSend();

                void PushOutQueue(PacketPtr &&Packet);

                TcpConnectionPtr connection_;

                uint8_t state_{kRtmpHandshake};

                RtmpHandshake handshake_;

                RtmpCallBack *rtmp_callback_{nullptr};

                // 上一个chunk stream id， csid， 的消息头 下同
                std::unordered_map<uint32_t, RtmpMsgHeaderPtr> in_message_headers_;

                // 保存与上一个时间戳的差值
                std::unordered_map<uint32_t, uint32_t> in_deltas_;

                std::unordered_map<uint32_t, PacketPtr> in_packet_;

                // 指定csid是有具有扩展时间戳
                std::unordered_map<uint32_t, bool> in_ext_;

                int in_chunk_size_{128};


                // message 发送相关

                // 当前需要chunk的数据缓冲区
                char out_buffer_[MESSAGE_CHUNK_SIZE];

                // 当前待发送的数据的起始位置
                char *out_current_{nullptr};

                // csid 对应的 时间戳差
                std::unordered_map<uint32_t, uint32_t> out_deltas_;

                // 一个csid 之前的一个头部信息
                std::unordered_map<uint32_t, RtmpMsgHeaderPtr> out_message_headers_;

                int32_t out_chunk_size_{MESSAGE_CHUNK_SIZE};

                // 待发送的数据缓 待处理为BufferNodePtr
                std::list<PacketPtr> out_waiting_queue;

                // 发送中的数据
                std::list<BufferNodePtr> sending_bufs_;

                // 保存已经传递发送数据的 生命周期
                std::list<PacketPtr> out_sending_packets_;

                // 发送状态
                bool sending_{false};


                // rtmp 协议控制消息
                // 确认窗口大小
                int32_t ack_size_{2500000};

                // 接受到的数据
                int32_t in_bytes_{0};
                
                // 剩下的数据
                int32_t last_left_{0};

        };
        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    }
}
