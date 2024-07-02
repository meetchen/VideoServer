/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpContext.cpp
 * @Description  :  Imp RtmpContext
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-02 23:24:26
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#include "mmedia/rtmp/RtmpContext.h"
#include "mmedia/base/MMediaLog.h"

using namespace vdse::mmedia;

RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *hanlder, bool client = false)
:conneciton_(conn), handshake_(conn, client), rtmp_handler_(hanlder)
{

}

RtmpContext::~RtmpContext()
{
}

int32_t RtmpContext::Parse(MsgBuffer &buf)
{
    if (state_ == kRtmpHandshake)
    {
        int ret = handshake_.Handshake(buf);
        if (ret == 0)
        {
            state_ = kRtmpMessage;
            if (buf.ReadableBytes())
            {
                return Parse(buf);
            }
        }
        else if (ret == 2)
        {
            state_ = kRtmpWaitDone;
        }
        else if (ret == -1)
        {
            RTMP_ERROR << " handshake parse error ";
        }
    }
    else if (state_ == kRtmpWaitDone)
    {
        
    }
    else if (state_ == kRtmpMessage)
    {

    }
}

void RtmpContext::OnwriteComplete()
{
    if (state_ == kRtmpHandshake)
    {
        handshake_.WriteComplete();
    }
    else if (state_ == kRtmpWaitDone)
    {
        state_ = kRtmpMessage;
    }
    else if (state_ == kRtmpMessage)
    {
        uint8_t fmt;
        
    }
}

void RtmpContext::StartHandshake()
{
    handshake_.Start();
}

int32_t RtmpContext::ParseMessage(MsgBuffer &buf)
{
    
}

void RtmpContext::MessageComplete(PacketPtr &&data)
{

}