/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpContext.cpp
 * @Description  :  Imp RtmpContext
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-04 09:18:02
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#include "mmedia/rtmp/RtmpContext.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h" 
#include "mmedia/base/BytesWriter.h"
#include "mmedia/rtmp/amf/AMFObject.h"

using namespace vdse::mmedia;

RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpCallBack *hanlder, bool client)
:connection_(conn), handshake_(conn, client), rtmp_callback_(hanlder)
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
        auto r = ParseMessage(buf);
        last_left_ = buf.ReadableBytes();
        return r;
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
    
    in_bytes_ += (buf.ReadableBytes() - last_left_);
    SentBytesRecv();

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
    auto type = data -> PacketType();
    switch (type)
    {
        case kRtmpMsgTypeChunkSize:
        {
            HandleChunkSize(data);
            break;
        }
        case kRtmpMsgTypeBytesRead:
        {
            RTMP_TRACE << "message bytes read recv.";
            break;
        }        
        case kRtmpMsgTypeUserControl:
        {
            HandleUserMessage(data);
            break;
        }
        case kRtmpMsgTypeWindowACKSize:
        {
            HandleAckWindowSize(data);
            break;
        }
        case kRtmpMsgTypeAMF3Message:
        {
            HandleAMFMessage(data, true);
            break;
        }
        case kRtmpMsgTypeAMFMessage:
        {
            HandleAMFMessage(data, false);
            break;
        }
        default:
        {
            RTMP_ERROR << " not surpport message type:" << type;
            break;
        }
            
    }
}


bool RtmpContext::BuildChunk(const PacketPtr &packet, uint32_t timestamp, bool fmt0)
{
    // 获取头部
    auto header = packet->Ext<RtmpMsgHeader>();

    if (!header) return false;

    out_sending_packets_.emplace_back(packet);
    
    auto& pre = out_message_headers_[header -> cs_id];

    // 是否是具有时间间隔， 
    /**
     * 1、 fmt0不具有  时间差
     * 2、 有时间差， 必须有前一个节点，这样才能计算时间差
     * 3、 时间戳必须必之前一个时间戳大
     * 4、 得是同一个 数据流的
     */
    bool is_deltal = !fmt0 && !pre && timestamp >= pre -> timestamp && header -> msg_sid == pre -> msg_sid;

    if (pre)
    {
        pre = std::make_shared<RtmpMsgHeader>();
    }

    // 先确定是fmt 0123
    int fmt = kRtmpFmt0;
    if (is_deltal)
    {
        fmt = kRtmpFmt1;
        // 长度 和 类型相同
        if (header -> msg_len == pre -> msg_len && header -> msg_type == pre -> msg_type)
        {
            fmt = kRtmpFmt2;
            // deltas 相同
            timestamp -= pre -> timestamp;
            if (out_deltas_[header -> cs_id] == timestamp)
            {
                fmt = kRtmpFmt3;
            }
        }
    }

    // 开始填充缓冲区的数据 
    char *p = out_current_;

    // 填入 fmt 和 csid
    if (header -> cs_id < 64)
    {
        *p++ = (char)((fmt << 6) | header -> cs_id);
    }
    else if (header -> cs_id < 64 + 256)
    {
        *p++ = (char)(fmt << 6);
        *p++ = (char)(header -> cs_id - 64);
    }
    else
    {
        *p++ = (char)((fmt << 6) | 1);
        uint16_t temp = header -> cs_id - 64;
        memcpy(p, &temp, 2);
        p += 2;
    }


    // 此时ts已经是上个与上一个chunk的timestamp的差值
    auto ts = timestamp;
    if (timestamp >= 0xffffff)
    {
        ts = 0xffffff;
    }

    // 按不同的fmt 开始填写数据
    if (fmt == kRtmpFmt0)
    {
        p += BytesWriter::WriteUint24T(p, ts);
        p += BytesWriter::WriteUint24T(p, header -> msg_len);
        p += BytesWriter::WriteUint8T(p, header -> msg_type);
        memcpy(p, &header -> msg_sid, 4);
        p += 4;
        out_deltas_[header -> cs_id] = 0;
    }
    else if (fmt == kRtmpFmt1)
    {
        p += BytesWriter::WriteUint24T(p, ts);
        p += BytesWriter::WriteUint24T(p, header -> msg_len);
        p += BytesWriter::WriteUint8T(p, header -> msg_type);
        out_deltas_[header -> cs_id] = timestamp;
    }
    else if (fmt == kRtmpFmt2)
    {
        p += BytesWriter::WriteUint24T(p, ts);
        out_deltas_[header -> cs_id] = timestamp;
    }
    else if (fmt == kRtmpFmt3)
    {
        
    }

    // 如果有time delta ，则进行填写
    if (timestamp >= 0xffffff)
    {
        memcpy(p, &timestamp, 4);
        p += 4;
    }

    // 先发送头部数据
    auto nheader = std::make_shared<BufferNode>(out_buffer_, p - out_current_);
    sending_bufs_.push_back(std::move(nheader));
    out_current_ = p;

    // 更新前一个信息
    pre -> cs_id = header -> cs_id;
    pre -> msg_len = header -> msg_len;
    pre -> msg_sid = header -> msg_sid;
    pre -> msg_type = header -> msg_type;

    if (fmt == kRtmpFmt0)
    {
        pre -> timestamp = timestamp;
    }
    else
    {
        pre -> timestamp += timestamp;
    }

    // 准备发送消息体
    const char *body = packet -> Data();
    int32_t bytes_parsed = 0;

    while (true)
    {
        // 此次数据写入的地址
        const char * dest = body + bytes_parsed;

        // 此次数据写入的长度
        int32_t size = header -> msg_len - bytes_parsed;

        // 每次写的最大数据不能超过out_chunk_size_
        size = std::min(size, out_chunk_size_);

        auto node = std::make_shared<BufferNode>((void *)dest, size);
        // 将数据放置到发送队列
        sending_bufs_.push_back(std::move(node));

        if (bytes_parsed >= header -> msg_len)
        {
            break;
        }

        if (out_current_ - out_buffer_ > MESSAGE_CHUNK_SIZE)
        {
            RTMP_ERROR << "rtmp had no enough buff";
            break;
        }
        char *p = out_current_;

        // 发送一个头部 fmt3
        // 填入 fmt 和 csid
        if (header -> cs_id < 64)
        {
            *p++ = (char)((fmt << 6) | header -> cs_id);
        }
        else if (header -> cs_id < 64 + 256)
        {
            *p++ = (char)(fmt << 6);
            *p++ = (char)(header -> cs_id - 64);
        }
        else
        {
            *p++ = (char)((fmt << 6) | 1);
            uint16_t temp = header -> cs_id - 64;
            memcpy(p, &temp, 2);
            p += 2;
        }
        
        if (timestamp >= 0xffffff)
        {
            memcpy(p, &timestamp, 4);
            p += 4;
        }
        
        auto nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.push_back(std::move(nheader));

        out_current_ = p;
   
    }
    return true;
    
}

void RtmpContext::Send()
{
    if (sending_)
    {
        return;
    }
    sending_ = true;

    for (int i = 0; i < 10; i++)
    {
        if (out_waiting_queue.empty())
        {
            break;
        }
        BuildChunk(std::move(out_waiting_queue.front()));
        out_waiting_queue.pop_front(); 
    }
    connection_->Send(sending_bufs_);
}

bool RtmpContext::Ready() const
{
    return !sending_;
}


bool RtmpContext::BuildChunk(PacketPtr &&packet, uint32_t timestamp, bool fmt0)
{
    // 获取头部
    auto header = packet->Ext<RtmpMsgHeader>();

    if (!header) return false;

    
    auto& pre = out_message_headers_[header -> cs_id];

    // 是否是具有时间间隔， 
    /**
     * 1、 fmt0不具有  时间差
     * 2、 有时间差， 必须有前一个节点，这样才能计算时间差
     * 3、 时间戳必须必之前一个时间戳大
     * 4、 得是同一个 数据流的
     */
    bool is_deltal = !fmt0 && !pre && timestamp >= pre -> timestamp && header -> msg_sid == pre -> msg_sid;

    if (pre)
    {
        pre = std::make_shared<RtmpMsgHeader>();
    }

    // 先确定是fmt 0123
    int fmt = kRtmpFmt0;
    if (is_deltal)
    {
        fmt = kRtmpFmt1;
        // 长度 和 类型相同
        if (header -> msg_len == pre -> msg_len && header -> msg_type == pre -> msg_type)
        {
            fmt = kRtmpFmt2;
            // deltas 相同
            timestamp -= pre -> timestamp;
            if (out_deltas_[header -> cs_id] == timestamp)
            {
                fmt = kRtmpFmt3;
            }
        }
    }

    // 开始填充缓冲区的数据 
    char *p = out_current_;

    // 填入 fmt 和 csid
    if (header -> cs_id < 64)
    {
        *p++ = (char)((fmt << 6) | header -> cs_id);
    }
    else if (header -> cs_id < 64 + 256)
    {
        *p++ = (char)(fmt << 6);
        *p++ = (char)(header -> cs_id - 64);
    }
    else
    {
        *p++ = (char)((fmt << 6) | 1);
        uint16_t temp = header -> cs_id - 64;
        memcpy(p, &temp, 2);
        p += 2;
    }


    // 此时ts已经是上个与上一个chunk的timestamp的差值
    auto ts = timestamp;
    if (timestamp >= 0xffffff)
    {
        ts = 0xffffff;
    }

    // 按不同的fmt 开始填写数据
    if (fmt == kRtmpFmt0)
    {
        p += BytesWriter::WriteUint24T(p, ts);
        p += BytesWriter::WriteUint24T(p, header -> msg_len);
        p += BytesWriter::WriteUint8T(p, header -> msg_type);
        memcpy(p, &header -> msg_sid, 4);
        p += 4;
        out_deltas_[header -> cs_id] = 0;
    }
    else if (fmt == kRtmpFmt1)
    {
        p += BytesWriter::WriteUint24T(p, ts);
        p += BytesWriter::WriteUint24T(p, header -> msg_len);
        p += BytesWriter::WriteUint8T(p, header -> msg_type);
        out_deltas_[header -> cs_id] = timestamp;
    }
    else if (fmt == kRtmpFmt2)
    {
        p += BytesWriter::WriteUint24T(p, ts);
        out_deltas_[header -> cs_id] = timestamp;
    }
    else if (fmt == kRtmpFmt3)
    {
        
    }

    // 如果有time delta ，则进行填写
    if (timestamp >= 0xffffff)
    {
        memcpy(p, &timestamp, 4);
        p += 4;
    }

    // 先发送头部数据
    auto nheader = std::make_shared<BufferNode>(out_buffer_, p - out_current_);
    sending_bufs_.push_back(std::move(nheader));
    out_current_ = p;

    // 更新前一个信息
    pre -> cs_id = header -> cs_id;
    pre -> msg_len = header -> msg_len;
    pre -> msg_sid = header -> msg_sid;
    pre -> msg_type = header -> msg_type;

    if (fmt == kRtmpFmt0)
    {
        pre -> timestamp = timestamp;
    }
    else
    {
        pre -> timestamp += timestamp;
    }

    // 准备发送消息体
    const char *body = packet -> Data();
    int32_t bytes_parsed = 0;

    while (true)
    {
        // 此次数据写入的地址
        const char * dest = body + bytes_parsed;

        // 此次数据写入的长度
        int32_t size = header -> msg_len - bytes_parsed;

        // 每次写的最大数据不能超过out_chunk_size_
        size = std::min(size, out_chunk_size_);

        auto node = std::make_shared<BufferNode>((void *)dest, size);
        // 将数据放置到发送队列
        sending_bufs_.push_back(std::move(node));

        if (bytes_parsed >= header -> msg_len)
        {
            break;
        }

        if (out_current_ - out_buffer_ > MESSAGE_CHUNK_SIZE)
        {
            RTMP_ERROR << "rtmp had no enough buff";
            break;
        }
        char *p = out_current_;

        // 发送一个头部 fmt3
        // 填入 fmt 和 csid
        if (header -> cs_id < 64)
        {
            *p++ = (char)((fmt << 6) | header -> cs_id);
        }
        else if (header -> cs_id < 64 + 256)
        {
            *p++ = (char)(fmt << 6);
            *p++ = (char)(header -> cs_id - 64);
        }
        else
        {
            *p++ = (char)((fmt << 6) | 1);
            uint16_t temp = header -> cs_id - 64;
            memcpy(p, &temp, 2);
            p += 2;
        }
        
        if (timestamp >= 0xffffff)
        {
            memcpy(p, &timestamp, 4);
            p += 4;
        }
        
        auto nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.push_back(std::move(nheader));

        out_current_ = p;
   
    }
    out_sending_packets_.push_back(std::move(packet));

    return true;
}

void RtmpContext::CheckAfterSend()
{
    sending_ = false;
    out_current_ = out_buffer_;
    sending_bufs_.clear();
    out_sending_packets_.clear();

    if (!out_waiting_queue.empty())
    {
        Send();
    }
    else
    {
        rtmp_callback_->OnActive(connection_);
    }
}

void RtmpContext::PushOutQueue(PacketPtr &&packet)
{
    out_waiting_queue.emplace_back(std::move(packet));
    Send();
}


void RtmpContext::SendSetChunkSize()
{
    auto packet = Packet::NewPacker(32);
    auto header = packet->Ext<RtmpMsgHeader>();
    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 4;
        header -> timestamp = 0;    
        header -> msg_type = kRtmpMsgTypeChunkSize;
    }

    auto p = packet->Data();
    packet -> UpDatePackSize(BytesWriter::WriteUint32T(p, out_chunk_size_));
    RTMP_DEBUG << " send chunk size : " << out_chunk_size_ << " to host" << connection_ -> PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendAckWindowSize()
{
    auto packet = Packet::NewPacker(32);
    auto header = packet->Ext<RtmpMsgHeader>();
    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> timestamp = 0;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 4;
        header -> msg_type = kRtmpMsgTypeWindowACKSize;
    }

    auto p = packet->Data();
    packet -> UpDatePackSize(BytesWriter::WriteUint32T(p, ack_size_));
    RTMP_DEBUG << " send ack size : " << ack_size_ << " to host" << connection_ -> PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendSetPeerBandwidth()
{
    auto packet = Packet::NewPacker(32);
    auto header = packet->Ext<RtmpMsgHeader>();
    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 5;
        header -> timestamp = 0;
        header -> msg_type = kRtmpMsgTypeSetPeerBW;
    }

    auto p = packet->Data();
    p += BytesWriter::WriteUint32T(p, ack_size_);
    *p++ = 0x02;
    packet -> UpDatePackSize(5);
    RTMP_DEBUG << " send band width : " << ack_size_ << " to host" << connection_ -> PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::SentBytesRecv()
{
    if (in_bytes_ >= ack_size_)
    {
        auto packet = Packet::NewPacker(32);
        auto header = packet->Ext<RtmpMsgHeader>();
        if (header)
        {
            header -> cs_id = kRtmpCSIDCommand;
            header -> msg_sid = kRtmpMsID0;
            header -> msg_len = 4;
            header -> msg_type = kRtmpMsgTypeBytesRead;
            header -> timestamp = 0;
        }

        auto p = packet->Data();
        packet -> UpDatePackSize(BytesWriter::WriteUint32T(p, in_bytes_));
        // RTMP_DEBUG << " send ack size : " << ack_size_ << " to host" << connection_ -> PeerAddr().ToIpPort();
        PushOutQueue(std::move(packet));
        in_bytes_ = 0;
    }
    
}

void RtmpContext::SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2)
{
    auto packet = Packet::NewPacker(32);
    auto header = packet->Ext<RtmpMsgHeader>();
    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> timestamp = 0;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 6;
        header -> msg_type = kRtmpMsgTypeWindowACKSize;
    }

    auto p = packet->Data();
    auto temp = p;
    p += BytesWriter::WriteUint16T(p, nType);
    p += BytesWriter::WriteUint32T(p, value1);
    if (nType == kRtmpEventTypeSetBufferLength)
    {
        p += BytesWriter::WriteUint32T(p, value2);
        header -> msg_len += 4;
    }
    packet -> UpDatePackSize(header -> msg_len);
    RTMP_DEBUG << " send ctrl msg : "
                << " ntype :" << nType
                << " value1 :" << value1
                << " value2 " << value2
                << " to host" << connection_ -> PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::HandleChunkSize(PacketPtr& packet)
{
    if (packet -> PacketSize() < 4)
    {
        RTMP_ERROR << " handle chunk size errror, packet size : " << packet -> PacketSize() << connection_->PeerAddr().ToIpPort();
        return; 
    }

    auto size = BytesReader::ReadUint32T(packet->Data());

    RTMP_TRACE << " in chunk size change from : " << in_chunk_size_ << " to " << size;

    in_chunk_size_ = size;

}

void RtmpContext::HandleAckWindowSize(PacketPtr &packet)
{
    if (packet -> PacketSize() < 4)
    {
        RTMP_ERROR << " handle  ack error, packet size : " << packet -> PacketSize() << connection_->PeerAddr().ToIpPort();
        return; 
    }

    auto size = BytesReader::ReadUint32T(packet->Data());

    RTMP_TRACE << " in ack size change from : " << ack_size_ << " to " << size;

    ack_size_ = size;
}

void RtmpContext::HandleUserMessage(PacketPtr &packet)
{
    auto msg_len = packet->PacketSize();

    if (msg_len < 6)
    {
        RTMP_ERROR << " handle UserMessage error, packet size : " << packet -> PacketSize() << connection_->PeerAddr().ToIpPort();
        return; 
    }

    auto body = packet->Data();
    auto type = BytesReader::ReadUint16T(body);
    auto value = BytesReader::ReadUint24T(body + 2);

    switch(type)
    {
        case kRtmpEventTypeStreamBegin:
        {
            RTMP_TRACE << "recv stream begin value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            break;
        }
        case kRtmpEventTypeStreamEOF:
        {
            RTMP_TRACE << "recv stream eof value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            break;
        }   
        case kRtmpEventTypeStreamDry:
        {
            RTMP_TRACE << "recv stream dry value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            break;
        }
        case kRtmpEventTypeSetBufferLength:
        {
            RTMP_TRACE << "recv set buffer length value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            if(msg_len<10)
            {
                RTMP_ERROR << "invalid user control packet msg_len:" << packet->PacketSize()
                << " host:" << connection_->PeerAddr().ToIpPort();                
                return ;
            }
            break;
        }   
        case kRtmpEventTypeStreamsRecorded:
        {
            RTMP_TRACE << "recv stream recoded value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            break;
        }
        case kRtmpEventTypePingRequest:
        {
            RTMP_TRACE << "recv ping request value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            SendUserCtrlMessage(kRtmpEventTypePingResponse,value,0);
            break;
        }   
        case kRtmpEventTypePingResponse:
        {
            RTMP_TRACE << "recv ping response value" << value << " host:" << connection_->PeerAddr().ToIpPort();
            break;
        }
        default:
            break;
    }
}

void RtmpContext::HandleAMFMessage(PacketPtr &packet, bool amf3)
{
    RTMP_TRACE << " handler AMF message , " << connection_ -> PeerAddr().ToIpPort();
    int msg_len = packet -> PacketSize();
    auto body = packet -> Data();
    if (amf3)
    {
        body++;
        msg_len--;
    }

    AMFObject obj;
    if (obj.Decode(body, msg_len) < 0)
    {
        RTMP_ERROR << "amf message decode error";
        return;
    }
    obj.Dump();

}