/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 22:26:16
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-05 17:31:59
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
#include "mmedia/rtmp/amf/AMFObject.h"
#include <unordered_map>
#include <memory>

namespace vdse
{
    namespace mmedia
    {
        enum RtmpContextState
        {
            kRtmpHandshake = 0,
            kRtmpWatingDone,
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

        using CommandCallBack = std::function<void(AMFObject&)>;
        
        class RtmpContext
        {
            public:
                RtmpContext(const TcpConnectionPtr &conn, RtmpCallBack *hanlder, bool client = false);
                
                ~RtmpContext();

                int32_t Parse(MsgBuffer &buf);

                void OnWriteComplete();
         
                void StartHandshake();

                int32_t ParseMessage(MsgBuffer &buf);

                void MessageComplete(PacketPtr &&data);

                void HandleAMFMessage(PacketPtr &packet, bool);

                void DumpShake()
                {
                    RTMP_TRACE << ((int)(state_)) << " \n";
                }


                
                bool BuildChunk(const PacketPtr &packet, uint32_t timestamp = 0, bool fmt0 = false);

                void Send();
                
                bool Ready() const;

                /**
                 * @brief        :  拉流
                 * @param         {string} &url:
                 * @return        {*}
                **/
                void Play(const std::string &url);

                /**
                 * @brief        :  推流
                 * @param         {string} &url:
                 * @return        {*}
                **/                
                void Publish(const std::string &url) ;

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


                /**
                 * @brief        :  客户端发送连接请求命令 amf
                 * @return        {*}
                **/                
                void SendConnect();

                /**
                 * @brief        : 处理客户端的请求命令 
                 * @param         {AMFObject} &obj: 收到的客户端请求命令体
                 * @return        {*}
                **/                
                void HandleConnect(AMFObject &obj);


                void SendCreateStream();

                void HandleCreateStream(AMFObject &obj);

                /**
                 * @brief        :  通知netStream状态更新
                 * @param         {string} &level:state error waring
                 * @param         {string} &code: 规定的状态值
                 * @param         {string} &description: 人类可读的描述
                 * @return        {*}
                **/                
                void SendStatus(const std::string &level, const std::string &code, const std::string &description);

                /**
                 * @brief        :  客户端发送play命令
                 * @return        {*}
                **/                
                void SendPlay();

                /**
                 * @brief        : 服务端收到play命令后，发送控制流消息， 使用onstatus通知客户端 
                 * @param         {AMFObject} &obj:
                 * @return        {*}
                **/                
                void HandlePlay(AMFObject &obj);



                void ParseNameAndTcUrl();

                void SendPublish();
                void HandlePublish(AMFObject &obj);

                void HandleResult(AMFObject &obj);
                void HandleError(AMFObject &obj);
                void SetPacketType(PacketPtr &packet);

                TcpConnectionPtr connection_;

                int state_{kRtmpHandshake};

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
                std::list<PacketPtr> out_waiting_queue_;

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

                // 推流的项目名称
                std::string app_;
                // 推流的地址
                std::string tc_url_;
                // 流名
                std::string name_;
                // 一个流对应的会话名称
                std::string session_name_;
                // 推流地址所带的参数
                std::string param_;
                // 该流当前是播放的还是停止的
                bool is_player_{false};
                // std::unordered_map<std::string,CommandFunc> commands_;
                bool is_client_{false};

                std::unordered_map<std::string, CommandCallBack> commands_;
        };
        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    }
}
