/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 18:57:08
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-02 16:26:22
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpHandshake.h
 * @Description: rtmp握手实现
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <cstdint>
#include "network/net/TcpConnection.h"
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <random>

namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        const int kRtmpHandshakePacketSize = 1536;
        
        enum RtmpHandshakeState
        {
            kHandshakeInit = 1,
            // Client
            KHandshakePostC0C1 = 2,
            KHandshakeWaitS0S1 = 3,
            KHandshakePostC2 = 4,
            KHandshakeWaitS2 = 5,
            KHandshakeDoning = 6,
            KHandshakeDone = 7,

            // Server
            KHandshakeWaitC0C1 = 8,
            KHandshakePostS0S1 = 9,
            KHandshakePostS2 = 10,
            KHandshakeWaitC2 = 11,
        };

        class RtmpHandshake
        {
            public:
                RtmpHandshake(const TcpConnectionPtr &conn, bool client);
                ~RtmpHandshake() = default;
                void Start();

                /**
                 * @brief        : z状态机函数 
                 * @param         {MsgBuffer} &buf:
                 * @return        {*} 0:握手成功， 1:表示还有更多数据 2:正在完成握手 -1:出错
                **/                
                int32_t Handshake(MsgBuffer &buf);

                void WriteComplete();

            private:
                TcpConnectionPtr connection_;
                bool is_client_{true};
                std::mt19937 mt_;
                std::uniform_int_distribution<> rand_;

                bool is_complex_handshake_{true};
                uint8_t digest_[SHA256_DIGEST_LENGTH];
                uint8_t C1S1_[kRtmpHandshakePacketSize + 1];
                uint8_t C2S2_[kRtmpHandshakePacketSize];
                uint8_t state_{kHandshakeInit};

                int32_t CheckC1S1(const char *data, int bytes);
                void CreateC1S1();
                void SendC1S1();

                /**
                 * @brief        : 创建C2 or S2 
                 * @param         {char} *data: 需要使用的C1 or S1
                 * @param         {int} bytes: C1 or S1 长度
                 * @param         {int} offset: C1 or S1 中 digset偏移量
                 * @return        {*}
                **/                
                void CreateC2S2(const char *data, int bytes, int offset);
                bool CheckC2S2(const char *data, int bytes);
                void SendC2S2();

                inline uint8_t GenRandom() 
                {
                    return rand_(mt_);
                }

        };

    }
}