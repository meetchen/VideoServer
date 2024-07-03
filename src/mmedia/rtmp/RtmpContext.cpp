/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpContext.cpp
 * @Description  :  Imp RtmpContext
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-03 09:28:14
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#include "mmedia/rtmp/RtmpContext.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"

using namespace vdse::mmedia;

RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *hanlder, bool client)
:conneciton_(conn), handshake_(conn, client), rtmp_handler_(hanlder)
{

}

RtmpContext::~RtmpContext()
{
}

int32_t RtmpContext::Parse(MsgBuffer &buf)
{
    int32_t ret = -1;
    if (state_ == kRtmpHandshake)
    {
        ret = handshake_.Handshake(buf);
        if (ret == 0)
        {
            state_ = kRtmpMessage;
            if (buf.ReadableBytes() > 0)
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
        return ParseMessage(buf);
    }  

    return ret;
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
        
    }
}

void RtmpContext::StartHandshake()
{
    handshake_.Start();
}

int32_t RtmpContext::ParseMessage(MsgBuffer &buf)
{
    uint8_t fmt;
    uint32_t csid, msg_len = 0, msg_sid = 0, timestamp = 0;
    uint8_t msg_type = 0;
    uint32_t total_bytes = buf.ReadableBytes();
    int32_t parsed = 0;
    

    while (total_bytes > 1)
    {
        const char *pos = buf.Peek();

        // Basic Header
        
        // 首字节的前两位为版本号
        fmt = (*pos >> 6) & 0x03;
        // 首字节的后六位为csid
        csid = (*pos & 0x3f);

        parsed++;

        if (csid == 0)
        {   
            // 第一个字节后6位取值0，表示csid至少是64，第二个字节8位，表示[0,255]的取值范围
            // 真正的csid的值为64+[0,255]=[64,319]
            if (total_bytes < 2)
            {
                return 1;
            }
            csid = 64;
            csid += *((uint8_t*)(pos + parsed));
            parsed++;
        }
        else if (csid == 1)
        {
            // 第一个字节后6位取值1，表示csid至少是64，后面两个字节，表示[0,65535]的取值范围
            // 真正的csid的值为64+[0,65535]=[64,65599]
            if (total_bytes < 3)
            {
                return 1;
            }
            csid = 64;
            csid += *((uint8_t *)(pos + parsed));
            parsed++;
            // 网络字节序问题
            csid += *((uint8_t *)(pos + parsed)) * 256;
            parsed++;

            // csid = 64 + BytesReader::ReadUint16T(pos + parsed);
            // parsed += 2;
        }
        
        int32_t size = total_bytes - parsed;        
        // Message header
        if (size == 0 || (fmt == 0 && size < 11) || (fmt == 1 && size < 7) ||  (fmt == 2 && size < 3) )
        {
            return 1;
        }

        // msg 总长
        msg_len = 0;
        // 流id
        msg_sid = 0;
        msg_type = 0;
        timestamp = 0;
        int32_t ts = 0;
        
        RtmpMsgHeaderPtr& prev = in_message_headers_[csid];

        if (!prev)
        {   
            prev = std::make_shared<RtmpMsgHeader>();
        }

        if (fmt == kRtmpFmt0)
        {
            ts = BytesReader::ReadUint24T(pos + parsed);
            timestamp = ts;
            in_deltas_[csid] = 0;
            parsed += 3;

            msg_len = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;

            msg_type = BytesReader::ReadUint8T(pos + parsed);
            parsed += 1;
            // sid 是小端序
            memcpy((void *)&msg_sid, (void *)(pos + parsed), 4);
            parsed += 4;
        }
        else if (fmt == kRtmpFmt1)
        {
            ts = BytesReader::ReadUint24T(pos + parsed);
            in_deltas_[csid] = ts;
            timestamp = ts + prev -> timestamp;
            parsed += 3;
            msg_len = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;
            msg_type = BytesReader::ReadUint8T(pos + parsed);
            parsed += 1;
            msg_sid = prev -> msg_sid;
        }
        else if (fmt == kRtmpFmt2)
        {
            ts = BytesReader::ReadUint24T(pos + parsed);
            in_deltas_[csid] = ts;
            timestamp = ts + prev -> timestamp;
            parsed += 3;
            msg_len = prev -> msg_len;
            msg_sid = prev -> msg_sid;
            msg_type = prev -> msg_type;
        }
        else if (fmt == kRtmpFmt3)
        {
            msg_len = prev -> msg_len;
            timestamp = prev -> timestamp + in_deltas_[csid];
            msg_sid = prev -> msg_sid;
            msg_type = prev -> msg_type;
        }
        
        bool has_ext = (ts == 0xffffff);
        // 由于fmt3没有ts，所以考虑前一个chunk有没有扩展时间戳
        if (fmt == kRtmpFmt3)
        {
            has_ext = in_ext_[csid];
        }

        in_ext_[csid] = has_ext;

        if (has_ext)
        {
            // 剩下的数据不够 ext
            if (total_bytes - parsed < 4)
            {
                return 1;
            }

            ts = BytesReader::ReadUint32T(pos + parsed);
            parsed += 4;
            if (fmt != kRtmpFmt0)
            {
                timestamp = ts + prev -> timestamp;
                in_deltas_[csid] = ts;
            }
        }

        auto& packet = in_packet_[csid];
        if (!packet)
        {
            packet = Packet::NewPacker(msg_len);
        }

        RtmpMsgHeaderPtr header = packet->Ext<RtmpMsgHeader>();

        if (!header)
        {
            header = std::make_shared<RtmpMsgHeader>();
            packet -> SetExt(header);
        }

        header -> cs_id = csid;
        header -> msg_len = msg_len;
        header -> msg_type = msg_type;
        header -> msg_sid = msg_sid;
        header -> timestamp = timestamp;

        int need_copy_bytes = std::min(packet -> Space(), in_chunk_size_);
        if (total_bytes - parsed < need_copy_bytes)
        {
            return 1;
        }

        auto data_dist = packet -> Data() + packet -> PacketSize();

        memcpy(data_dist, pos + parsed, need_copy_bytes);
        packet -> UpDatePackSize(need_copy_bytes);
        parsed += need_copy_bytes;
        buf.Retrieve(parsed);
        total_bytes -= parsed;

        prev -> cs_id = csid;
        prev -> msg_len = msg_len;
        prev -> msg_type = msg_type;
        prev -> msg_sid = msg_sid;
        prev -> timestamp = timestamp;
        
        // 没有空闲存放数据了，考虑就把这个包发出去了
        if (packet -> Space() == 0)
        {
            packet -> SetPacketType(msg_type);
            packet -> SetTimeStamp(timestamp);
            MessageComplete(std::move(packet));
            packet.reset();
        }
    }
    return 1;
}

void RtmpContext::MessageComplete(PacketPtr &&data)
{
    RTMP_TRACE << "recv message type " << data -> PacketType() << " len: " << data -> PacketSize() << std::endl;
}