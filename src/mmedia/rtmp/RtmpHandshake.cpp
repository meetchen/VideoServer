/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 19:03:22
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-01 20:04:23
 * @FilePath: /VideoServer/src/mmedia/rtmp/RtmpHandshake.cpp
 * @Description: Imp RtmpHandshake
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/TcpConnection.h"
#include "mmedia/rtmp/RtmpHandshake.h"
#include "mmedia/base/MMediaLog.h"
#include "base/TTime.h"
#include <string>
#include <memory>
#include <cstdint>


#if OPENSSL_VERSION_NUMBER > 0x10100000L
#define HMAC_setup(ctx, key, len)	ctx = HMAC_CTX_new(); HMAC_Init_ex(ctx, key, len, EVP_sha256(), 0)
#define HMAC_crunch(ctx, buf, len)	HMAC_Update(ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen)	HMAC_Final(ctx, dig, &dlen); HMAC_CTX_free(ctx)
#else
#define HMAC_setup(ctx, key, len)	HMAC_CTX_init(&ctx); HMAC_Init_ex(&ctx, key, len, EVP_sha256(), 0)
#define HMAC_crunch(ctx, buf, len)	HMAC_Update(&ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen)	HMAC_Final(&ctx, dig, &dlen); HMAC_CTX_cleanup(&ctx)
#endif 

namespace
{
    static const unsigned char rtmp_server_ver[4] = 
    {
        0x0D, 0x0E, 0x0A, 0x0D
    };

    static const unsigned char rtmp_client_ver[4] = 
    {
        0x0C, 0x00, 0x0D, 0x0E
    };

    #define PLAYER_KEY_OPEN_PART_LEN 30   ///< length of partial key used for first client digest signing
    /** Client key used for digest signing */
    static const uint8_t rtmp_player_key[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ', '0', '0', '1',

        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02,
        0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8,
        0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
    };

    #define SERVER_KEY_OPEN_PART_LEN 36   ///< length of partial key used for first server digest signing
    /** Key used for RTMP server digest signing */
    static const uint8_t rtmp_server_key[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
        'S', 'e', 'r', 'v', 'e', 'r', ' ', '0', '0', '1',

        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02,
        0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8,
        0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
    };

    /**
     * @brief        :  计算信息摘要，并存放到指定位置
     * @param         {uint8_t} *src: 全部数据
     * @param         {int} len: 数据长度
     * @param         {int} gap: offset存放地址偏移量
     * @param         {uint8_t} *key: 秘钥
     * @param         {int} keylen: 秘钥长度
     * @param         {uint8_t} *dst: offset存放目的地址
     * @return        {*}
    **/    
    void CalculateDigest(const uint8_t *src, int len, int gap,const uint8_t *key, int keylen, uint8_t *dst)
    {
        uint32_t digestLen=0;
        #if OPENSSL_VERSION_NUMBER > 0x10100000L    
        HMAC_CTX *ctx;
        #else
        HMAC_CTX ctx;
        #endif    
        HMAC_setup(ctx, key, keylen);
        if(gap<=0)
        {
            HMAC_crunch(ctx, src, len);
        }        
        else
        {                     
            HMAC_crunch(ctx, src, gap);
            HMAC_crunch(ctx, src+gap+SHA256_DIGEST_LENGTH, len-gap-SHA256_DIGEST_LENGTH);
        }
        HMAC_finish(ctx, dst, digestLen);
    } 

    bool VerifyDigest(uint8_t *buf, int digest_pos,const uint8_t *key, size_t keyLen)
    {
        uint8_t digest[SHA256_DIGEST_LENGTH];
        CalculateDigest(buf,1536, digest_pos, key, keyLen, digest);

        return memcmp(&buf[digest_pos], digest,SHA256_DIGEST_LENGTH) == 0;
    } 

    /**
     * @brief        :  计算digest中 digest-data的偏移量
     * @param         {uint8_t} *buf:整个digest起始位置
     * @param         {int} off: offset量
     * @param         {int} mod_val: 保证得到的offset是一个合法值 764 - 4(offset) - 32(digest-data) = 728
     * @return        {*}
    **/    
    int32_t GetDigestOffset(const uint8_t *buf, int off, int mod_val)
    {
        uint32_t offset = 0;
        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(buf + off);
        uint32_t res;

        offset = ptr[0]+ptr[1]+ptr[2]+ptr[3];
        res = (offset % mod_val) + (off+4);
        return res;
    }

}

using namespace vdse::mmedia;

RtmpHandshake::RtmpHandshake(const TcpConnectionPtr &conn, bool client)
:connection_(conn), 
is_client_(client), 
mt_(std::random_device{}()), 
rand_(0, 255)
{

}

void RtmpHandshake::Start()
{
    CreateC1S1();
    if (is_client_)
    {
        state_ = KHandshakePostC0C1;
        SendC1S1();
    }
    else
    {
        state_ = KHandshakeWaitC0C1;
    }
}

// uint8_t RtmpHandshake::GenRandom()
// {
    // // 梅森旋转随机数生成器，一种高质量的伪随机数生成器
    // // random_device 一个用于生成种子的设备，通常用于生成真正的随机数（如果硬件支持）
    // std::mt19937 mt{std::random_device{}()};
    // // 一个均匀分布的整数分布器，用于生成指定范围内的随机整数
    // std::uniform_int_distribution<> rand(0, 255);
    // return rand(mt);
// }

void RtmpHandshake::CreateC1S1()
{
    for (int i = 0; i <= kRtmpHandshakePacketSize; i++)
    {
        C1S1_[i] = GenRandom();
    }

    // '\x' 转义字符 表示后面跟着两个十六进制位
    C1S1_[0] = '\x03';

    // time 字段 可以为 zero or 任意值
    memset(C1S1_ + 1, 0x00, 4);

    if (is_complex_handshake_)
    {   
        // 计算在digest结构中， digest-data的偏移量，参数含义看方法注释
        auto offset = GetDigestOffset(C1S1_ + 1, 8, 768 - 4 - 32);
        uint8_t *data = C1S1_ + 1 + offset;
        // version 字段
        if (is_client_)
        {
            memcpy(C1S1_ + 5, rtmp_client_ver, 4);
            CalculateDigest(C1S1_ + 1, kRtmpHandshakePacketSize, offset, rtmp_player_key ,PLAYER_KEY_OPEN_PART_LEN, data);
        }
        else
        {
            memcpy(C1S1_ + 5, rtmp_server_ver, 4);
            CalculateDigest(C1S1_ + 1, kRtmpHandshakePacketSize, offset, rtmp_server_key ,SERVER_KEY_OPEN_PART_LEN, data);

        }
        memcpy(digest_, data, SHA256_DIGEST_LENGTH);
    }
    else
    {
        // zero 字段
        memset(C1S1_ + 5, 0x00, 4);
    }

}

int32_t RtmpHandshake::CheckC1S1(const char *data, int bytes)
{
    if (bytes != kRtmpHandshakePacketSize + 1)
    {
        RTMP_ERROR << "unexpected c1s1, len = " << bytes;
        return -1;
    }

    if (data[0] != '\x03')
    {
        RTMP_ERROR << "unexpected c1s1, version = " << data[0];
        return -1;
    }

    uint32_t offset = -1;

    // 跳过c0 跳过timestamp
    uint32_t *version = (uint32_t *)(data + 5);
    if (*version == 0)
    {
        is_complex_handshake_ = false;
        return 0;
    }

    if (is_complex_handshake_)
    {
        uint8_t *hankshake = (uint8_t *)(data + 1);
        offset = GetDigestOffset(hankshake, 8, 728);

        if (!is_client_)
        {
            if (!VerifyDigest(hankshake, offset, rtmp_player_key, SERVER_KEY_OPEN_PART_LEN))
            {
                // digset在后的计算方式
                offset = GetDigestOffset(hankshake, 772, 728);
                if(!VerifyDigest(hankshake, offset, rtmp_player_key, SERVER_KEY_OPEN_PART_LEN))
                {
                    return -1;
                }
            }
        }
        else
        {
            if (!VerifyDigest(hankshake, offset, rtmp_client_ver, PLAYER_KEY_OPEN_PART_LEN))
            {
                // digset在后的计算方式
                offset = GetDigestOffset(hankshake, 772, 728);
                if(!VerifyDigest(hankshake, offset, rtmp_client_ver, PLAYER_KEY_OPEN_PART_LEN))
                {
                    return -1;
                }
            }
        }
    }

    return offset;
}

void RtmpHandshake::SendC1S1()
{
    connection_->Send((const char *)C1S1_, kRtmpHandshakePacketSize + 1);
}

void RtmpHandshake::CreateC2S2(const char *data, int bytes, int offset)
{
    for (int i = 0; i < kRtmpHandshakePacketSize; i++)
    {
        C2S2_[i] = GenRandom();
    }

    memcpy(C2S2_, data, 8);

    auto timestamp = vdse::base::TTime::Now();
    char *t = (char *)&timestamp;
    C2S2_[3] = t[0];
    C2S2_[2] = t[1];
    C2S2_[1] = t[2];
    C2S2_[0] = t[3];

    if (is_complex_handshake_)
    {
        uint8_t new_key_from_c1s1_digest[32];
        if (is_client_)
        {
            CalculateDigest(digest_, 32, 0, rtmp_server_key, sizeof(rtmp_server_key), new_key_from_c1s1_digest);
        }
        else
        {
            CalculateDigest(digest_, 32, 0, rtmp_player_key, sizeof(rtmp_player_key), new_key_from_c1s1_digest);
        }
        CalculateDigest(C2S2_, kRtmpHandshakePacketSize - 32, 0, new_key_from_c1s1_digest, 32, &C2S2_[kRtmpHandshakePacketSize - 32]);
    }

}

bool RtmpHandshake::CheckC2S2(const char *data, int bytes)
{
    return true;
}

void RtmpHandshake::SendC2S2()
{
    connection_->Send((const char *)C2S2_, kRtmpHandshakePacketSize);
}

int32_t RtmpHandshake::Handshake(MsgBuffer &buf)
{
    switch(state_)
    {
        case KHandshakeWaitC0C1:
        {
            if (buf.ReadableBytes() < kRtmpHandshakePacketSize + 1)
            {
                return -1;
            }
            auto offset = CheckC1S1(buf.Peek(), kRtmpHandshakePacketSize + 1);
            if (offset >= 0)
            {
                CreateC2S2(buf.Peek() + 1, kRtmpHandshakePacketSize, offset);
                buf.Retrieve(kRtmpHandshakePacketSize + 1);
                state_ = KHandshakePostS0S1;
                SendC1S1();
            }
            else
            {
                RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << ", check C0C1 error";
                return -1;
            }
            break;
        }
        case KHandshakeWaitC2:
        {
            if (buf.ReadableBytes() < kRtmpHandshakePacketSize)
            {
                return -1;
            }
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << ", recv c2 ";
            if (CheckC2S2(buf.Peek(), kRtmpHandshakePacketSize))
            {
                buf.Retrieve(kRtmpHandshakePacketSize);
                RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << ", done. ";
                state_ = KHandshakeDone;
            }
            else
            {   
                RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << ", check C2 error";
                return -1;
            }
            break;
        }

        case KHandshakeWaitS0S1:
        {
            if (buf.ReadableBytes() < kRtmpHandshakePacketSize + 1)
            {
                return -1;
            }
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << ", recv SOS1 ";

            auto offset = CheckC1S1(buf.Peek(), kRtmpHandshakePacketSize + 1);
            if (offset >= 0)
            {
                CreateC2S2(buf.Peek() + 1, kRtmpHandshakePacketSize, offset);
                buf.Retrieve(kRtmpHandshakePacketSize + 1);
                // 在接受到S0S1的同时接受到了S2
                if (buf.ReadableBytes() == 1536)
                {
                    state_ = KHandshakeDoning;
                    SendC2S2();
                    return 2;
                }
            
                state_ = KHandshakePostC2;
                SendC2S2();
            }
            else
            {
                RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << ", check SOS1 error";
                return -1;
            }
            break;
        }
    }
    return 1;
}

void RtmpHandshake::WriteComplete()
{
    switch(state_)
    {
        case KHandshakePostS0S1:
        {
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << " PostS0S1";

            state_ = KHandshakePostS2;
            SendC2S2();
            break;
        }
        case KHandshakePostS2:
        {
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << " PostS2";

            state_ = KHandshakeWaitC2;
            break;
        }
        case KHandshakePostC0C1:
        {
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << " PostC0C1";

            state_ = KHandshakeWaitS0S1;
            break;
        }
        case KHandshakePostC2:
        {
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << " PostC2";

            state_ =KHandshakeDone;
            break;
        }
        case KHandshakeDoning:
        {
            RTMP_TRACE << " host " << connection_ -> PeerAddr().ToIpPort() << " PostC2";
            state_ =KHandshakeDone;
            break;
        }
    }
}