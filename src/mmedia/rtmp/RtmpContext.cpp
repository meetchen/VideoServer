/**
 * @FilePath     : /VideoServer/src/mmedia/rtmp/RtmpContext.cpp
 * @Description  :  Imp RtmpContext
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-17 00:42:44
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "mmedia/rtmp/RtmpContext.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h" 
#include "mmedia/base/BytesWriter.h"
#include "base/StringUtils.h"
#include "mmedia/rtmp/amf/AMFObject.h"
#include <thread>

using namespace vdse::mmedia;

RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpCallBack *hanlder, bool client)
:connection_(conn), handshake_(conn, client), rtmp_callback_(hanlder), is_client_(client)
{
    commands_["connect"] = std::bind(&RtmpContext::HandleConnect,this,std::placeholders::_1);
    commands_["createStream"] = std::bind(&RtmpContext::HandleCreateStream,this,std::placeholders::_1);
    commands_["_result"] = std::bind(&RtmpContext::HandleResult,this,std::placeholders::_1);
    commands_["_error"] = std::bind(&RtmpContext::HandleError,this,std::placeholders::_1);
    commands_["play"] = std::bind(&RtmpContext::HandlePlay,this,std::placeholders::_1);
    commands_["publish"] = std::bind(&RtmpContext::HandlePublish,this,std::placeholders::_1);
    out_current_ = out_buffer_;
}

RtmpContext::~RtmpContext()
{
}

int32_t RtmpContext::Parse(MsgBuffer &buf)
{
    // RTMP_TRACE << "RtmpContext::Parse(MsgBuffer &buf) state : " << state_ ;
    int32_t ret = -1;
    if (state_ == kRtmpHandshake)
    {
        ret = handshake_.Handshake(buf);
        if (ret == 0)
        { 
            state_ = kRtmpMessage;
            // 如果是客户端 发送连接
            if(is_client_)
            {
                SendConnect();
            }
            if (buf.ReadableBytes() > 0)
            {
                RTMP_TRACE << " still has readable bytes : " << buf.ReadableBytes()  ;
                return Parse(buf);
            }
        }
        else if (ret == 2)
        {
            state_ = kRtmpWatingDone;
        }
        else if (ret == -1)
        {
            RTMP_ERROR << " handshake parse error ";
        }
    }
    else if (state_ == kRtmpWatingDone)
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

void RtmpContext::OnWriteComplete()
{
    RTMP_TRACE << "RtmpContext::OnWriteComplete() , state_ =  " << state_ ;
    if (state_ == kRtmpHandshake)
    {
        handshake_.WriteComplete();
    }
    else if (state_ == kRtmpWatingDone)
    {
        state_ = kRtmpMessage;
        if (is_client_)
        {
            SendConnect();
        }
    }
    else if (state_ == kRtmpMessage)
    {
        CheckAfterSend();
    }
}

void RtmpContext::StartHandshake()
{
    handshake_.Start();
}
int32_t RtmpContext::ParseMessage(MsgBuffer &buf)
{
    uint8_t fmt;
    uint32_t csid,msg_len=0,msg_sid=0;
    uint8_t msg_type = 0;
    uint32_t total_bytes = buf.ReadableBytes();
    int32_t parsed = 0;

    in_bytes_ += (buf.ReadableBytes()-last_left_);
    SentBytesRecv();
    
    while(total_bytes>1)
    {
        const char *pos = buf.Peek();
        parsed = 0;
        //Basic Header
        fmt = (*pos>>6)&0x03;
        csid = *pos&0x3F;
        parsed++;

        if(csid == 0)
        {
            if(total_bytes<2)
            {
                return 1;
            }
            csid = 64;
            csid += *((uint8_t*)(pos+parsed));
            parsed++;
        }
        else if( csid == 1)
        {
            if(total_bytes<3)
            {
                return 1;
            }
            csid = 64;
            csid += *((uint8_t*)(pos+parsed));
            parsed++;
            csid +=  *((uint8_t*)(pos+parsed))*256;
            parsed ++;           
        }

        int size = total_bytes - parsed;
        if(size == 0
            ||(fmt==0&&size<11)
            ||(fmt==1&&size<7)
            ||(fmt==2&&size<3))
        {
            return 1;
        }

        msg_len = 0;
        msg_sid = 0;
        msg_type = 0;
        int32_t ts = 0;

        RtmpMsgHeaderPtr &prev = in_message_headers_[csid];
        if(!prev)
        {
            prev = std::make_shared<RtmpMsgHeader>();
        }
        msg_len = prev->msg_len;
        if(fmt == kRtmpFmt0 || fmt == kRtmpFmt1)
        {
            msg_len = BytesReader::ReadUint24T((pos+parsed)+3);
        }
        else if(msg_len == 0)
        {
            msg_len = in_chunk_size_;
        }
        PacketPtr &packet = in_packet_[csid];
        if(!packet)
        {
            packet = Packet::NewPacket(msg_len);
            RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
            header->cs_id = csid;
            header->msg_len = msg_len;
            header->msg_sid = msg_sid;
            header->msg_type = msg_type;
            header->timestamp = 0;  
            packet->SetExt(header);          
        }

        RtmpMsgHeaderPtr header = packet->Ext<RtmpMsgHeader>();

        if(fmt == kRtmpFmt0)
        {
            ts = BytesReader::ReadUint24T(pos+parsed);
            parsed += 3;
            in_deltas_[csid] = 0;
            header->timestamp = ts;
            header->msg_len = BytesReader::ReadUint24T(pos+parsed);
            parsed += 3;
            header->msg_type = BytesReader::ReadUint8T(pos+parsed);
            parsed += 1;
            memcpy(&header->msg_sid,pos+parsed,4);
            parsed += 4;
        }
        else if(fmt == kRtmpFmt1)
        {
            ts = BytesReader::ReadUint24T(pos+parsed);
            parsed += 3;
            in_deltas_[csid] = ts;
            header->timestamp = ts + prev->timestamp;
            header->msg_len = BytesReader::ReadUint24T(pos+parsed);
            parsed += 3;
            header->msg_type = BytesReader::ReadUint8T(pos+parsed);
            parsed += 1;
            header->msg_sid = prev->msg_sid;
        }
        else if(fmt == kRtmpFmt2)
        {
            ts = BytesReader::ReadUint24T(pos+parsed);
            parsed += 3;
            in_deltas_[csid] = ts;
            header->timestamp = ts + prev->timestamp;
            header->msg_len = prev->msg_len;
            header->msg_type = prev->msg_type;
            header->msg_sid = prev->msg_sid;
        }    
        else if(fmt == kRtmpFmt3)
        {
            if(header->timestamp == 0)
            {
                header->timestamp = in_deltas_[csid] + prev->timestamp;
            }
            header->msg_len = prev->msg_len;
            header->msg_type = prev->msg_type;
            header->msg_sid = prev->msg_sid;
        } 

        bool ext = (ts == 0xFFFFFF);
        if(fmt == kRtmpFmt3)
        {
            ext = in_ext_[csid];
        }
        in_ext_[csid] = ext;
        if(ext)
        {
            if(total_bytes - parsed < 4)
            {
                return 1;
            }
            ts = BytesReader::ReadUint32T(pos+parsed);
            parsed += 4;
            if(fmt != kRtmpFmt0)
            {
                header->timestamp = ts+ prev->timestamp;
                in_deltas_[csid] = ts;
            }
        }

        int bytes = std::min(packet->Space(),in_chunk_size_);
        if(total_bytes - parsed < bytes)
        {
            return 1;
        }

        const char * body = packet->Data() + packet->PacketSize();
        memcpy((void*)body,pos+parsed,bytes);
        packet->UpDatePackSize(bytes);
        parsed += bytes;
        
        buf.Retrieve(parsed);
        total_bytes -= parsed;

        prev->cs_id = header->cs_id;
        prev->msg_len = header->msg_len;
        prev->msg_sid = header->msg_sid;
        prev->msg_type = header->msg_type;
        prev->timestamp = header->timestamp;

        if(packet->Space() == 0)
        {
            packet->SetPacketType(header->msg_type);
            packet->SetTimeStamp(header->timestamp);
            MessageComplete(std::move(packet));
            packet.reset();
        }
    }
    return 1;
}

/**
int32_t RtmpContext::ParseMessage(MsgBuffer &buf)
{
    uint8_t fmt;
    uint32_t csid = 0, msg_len = 0, msg_sid = 0, timestamp = 0;
    uint8_t msg_type = 0;
    uint32_t total_bytes = buf.ReadableBytes();
    int32_t parsed = 0;
    
    in_bytes_ += (buf.ReadableBytes() - last_left_);
    SentBytesRecv();

    while (total_bytes > 1)
    {
        const char *pos = buf.Peek();

        parsed = 0;

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
            packet = Packet::NewPacket(msg_len);
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
*/
void RtmpContext::MessageComplete(PacketPtr &&data)
{
    // RTMP_TRACE << "recv message type " << data -> PacketType() << " len: " << data -> PacketSize();
    
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
        case kRtmpMsgTypeAMFMeta:
        case kRtmpMsgTypeAMF3Meta:
        case kRtmpMsgTypeAudio:
        case kRtmpMsgTypeVideo:
        {
            SetPacketType(data);
            if(rtmp_callback_)
            {
                rtmp_callback_->OnRecv(connection_,std::move(data));
            }
            break;
        } 
        default:
        {
            RTMP_ERROR << " not surpport message type:" << type;
            break;
        }
            
    }
}


// bool RtmpContext::BuildChunk(const PacketPtr &packet, uint32_t timestamp, bool fmt0)
// {
//     // 获取头部
//     auto header = packet->Ext<RtmpMsgHeader>();

//     if (!header) return false;

//     out_sending_packets_.emplace_back(packet);
    
//     auto& pre = out_message_headers_[header -> cs_id];

//     // 是否是具有时间间隔， 
//     /**
//      * 1、 fmt0不具有  时间差
//      * 2、 有时间差， 必须有前一个节点，这样才能计算时间差
//      * 3、 时间戳必须必之前一个时间戳大
//      * 4、 得是同一个 数据流的
//      */
//     bool is_deltal = !fmt0 && pre && timestamp >= pre -> timestamp && header -> msg_sid == pre -> msg_sid;

//     if (!pre)
//     {
//         pre = std::make_shared<RtmpMsgHeader>();
//     }

//     // 先确定是fmt 0123
//     int fmt = kRtmpFmt0;
//     if (is_deltal)
//     {
//         fmt = kRtmpFmt1;
//         // 长度 和 类型相同
//         if (header -> msg_len == pre -> msg_len && header -> msg_type == pre -> msg_type)
//         {
//             fmt = kRtmpFmt2;
//             // deltas 相同
//             timestamp -= pre -> timestamp;
//             if (out_deltas_[header -> cs_id] == timestamp)
//             {
//                 fmt = kRtmpFmt3;
//             }
//         }
//     }

//     // 开始填充缓冲区的数据 
//     char *p = out_current_;

//     // 填入 fmt 和 csid
//     if (header -> cs_id < 64)
//     {
//         *p++ = (char)((fmt << 6) | header -> cs_id);
//     }
//     else if (header -> cs_id < 64 + 256)
//     {
//         *p++ = (char)((fmt << 6) | 0);
//         *p++ = (char)(header -> cs_id - 64);
//     }
//     else
//     {
//         *p++ = (char)((fmt << 6) | 1);
//         uint16_t temp = header -> cs_id - 64;
//         memcpy(p, &temp, 2);
//         p += 2;
//     }


//     // 此时ts已经是上个与上一个chunk的timestamp的差值
//     auto ts = timestamp;
//     if (timestamp >= 0xffffff)
//     {
//         ts = 0xffffff;
//     }

//     // 按不同的fmt 开始填写数据
//     if (fmt == kRtmpFmt0)
//     {
//         p += BytesWriter::WriteUint24T(p, ts);
//         p += BytesWriter::WriteUint24T(p, header -> msg_len);
//         p += BytesWriter::WriteUint8T(p, header -> msg_type);
//         memcpy(p, &header -> msg_sid, 4);
//         p += 4;
//         out_deltas_[header -> cs_id] = 0;
//     }
//     else if (fmt == kRtmpFmt1)
//     {
//         p += BytesWriter::WriteUint24T(p, ts);
//         p += BytesWriter::WriteUint24T(p, header -> msg_len);
//         p += BytesWriter::WriteUint8T(p, header -> msg_type);
//         out_deltas_[header -> cs_id] = timestamp;
//     }
//     else if (fmt == kRtmpFmt2)
//     {
//         p += BytesWriter::WriteUint24T(p, ts);
//         out_deltas_[header -> cs_id] = timestamp;
//     }
//     else if (fmt == kRtmpFmt3)
//     {
        
//     }

//     // 如果有time delta ，则进行填写
//     if (timestamp >= 0xffffff)
//     {
//         memcpy(p, &timestamp, 4);
//         p += 4;
//     }

//     // 先发送头部数据
//     auto nheader = std::make_shared<BufferNode>(out_buffer_, p - out_current_);
//     sending_bufs_.push_back(std::move(nheader));
//     out_current_ = p;

//     // 更新前一个信息
//     pre -> cs_id = header -> cs_id;
//     pre -> msg_len = header -> msg_len;
//     pre -> msg_sid = header -> msg_sid;
//     pre -> msg_type = header -> msg_type;

//     if (fmt == kRtmpFmt0)
//     {
//         pre -> timestamp = timestamp;
//     }
//     else
//     {
//         pre -> timestamp += timestamp;
//     }

//     // 准备发送消息体
//     const char *body = packet -> Data();
//     int32_t bytes_parsed = 0;

//     while (true)
//     {
//         // 此次数据写入的地址
//         const char * dest = body + bytes_parsed;

//         // 此次数据写入的长度
//         int32_t size = header -> msg_len - bytes_parsed;

//         // 每次写的最大数据不能超过out_chunk_size_
//         size = std::min(size, out_chunk_size_);

//         auto node = std::make_shared<BufferNode>((void *)dest, size);
        
//         // 将数据放置到发送队列
//         sending_bufs_.push_back(std::move(node));

//         bytes_parsed += size;

//         if (bytes_parsed >= header -> msg_len)
//         {
//             break;
//         }

//         if (out_current_ - out_buffer_ > MESSAGE_CHUNK_SIZE)
//         {
//             RTMP_ERROR << "rtmp had no enough buff : out_current_ :" << out_current_ << " out_buffer_ " << out_buffer_;
//             break;
//         }
//         char *p = out_current_;

//         // 发送一个头部 fmt3
//         // 填入 fmt 和 csid
//         if (header -> cs_id < 64)
//         {
//             *p++ = (char)((0xc0) | header -> cs_id);
//         }
//         else if (header -> cs_id < 64 + 256)
//         {
//             *p++ = (char)(0xc0 | 0);
//             *p++ = (char)(header -> cs_id - 64);
//         }
//         else
//         {
//             *p++ = (char)((0xc0) | 1);
//             uint16_t temp = header -> cs_id - 64;
//             memcpy(p, &temp, 2);
//             p += 2;
//         }
        
//         if (timestamp >= 0xffffff)
//         {
//             memcpy(p, &timestamp, 4);
//             p += 4;
//         }
        
//         auto nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);

//         // 增加生命周期
//         sending_bufs_.push_back(std::move(nheader));

//         out_current_ = p;
   
//     }
//     return true;
    
// }


bool RtmpContext::BuildChunk(const PacketPtr &packet,uint32_t timestamp,bool fmt0)
{
    RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
    if(h)
    {
        out_sending_packets_.emplace_back(packet);
        RtmpMsgHeaderPtr &prev = out_message_headers_[h->cs_id];
        bool use_delta = !fmt0 && !prev && timestamp >= prev->timestamp && h->msg_sid == prev->msg_sid;
        if(!prev)
        {
            prev = std::make_shared<RtmpMsgHeader>();
        }
        int fmt = kRtmpFmt0;
        if(use_delta)
        {
            fmt = kRtmpFmt1 ;
            timestamp -= prev->timestamp;
            if(h->msg_type == prev->msg_type
                && h->msg_len == prev->msg_len)
            {
                fmt = kRtmpFmt2;
                if(timestamp == out_deltas_[h->cs_id]) 
                {
                    fmt = kRtmpFmt3;
                }   
            }
        }

        char *p = out_current_;

        if(h->cs_id<64)
        {
            *p++ = (char)((fmt<<6)|h->cs_id);
        }
        else if(h->cs_id<(64+256))
        {
           *p++ = (char)((fmt<<6)|0); 
           *p++ = (char)(h->cs_id - 64);
        }
        else 
        {
           *p++ = (char)((fmt<<6)|1); 
           uint16_t cs = h->cs_id-64;
           memcpy(p,&cs,sizeof(uint16_t));
           p += sizeof(uint16_t);
        }

        auto ts = timestamp;
        if(timestamp >= 0xFFFFFF)
        {
            ts = 0xFFFFFF;
        }

        if(fmt == kRtmpFmt0)
        {
            p += BytesWriter::WriteUint24T(p,ts);
            p += BytesWriter::WriteUint24T(p,h->msg_len);
            p += BytesWriter::WriteUint8T(p,h->msg_type);

            memcpy(p,&h->msg_sid,4);
            p += 4;
            out_deltas_[h->cs_id] = 0;
        } 
        else if(fmt == kRtmpFmt1)
        {
            p += BytesWriter::WriteUint24T(p,ts);
            p += BytesWriter::WriteUint24T(p,h->msg_len);
            p += BytesWriter::WriteUint8T(p,h->msg_type);
            out_deltas_[h->cs_id] = timestamp;
        }
        else if(fmt == kRtmpFmt2)
        {
            p += BytesWriter::WriteUint24T(p,ts);
            out_deltas_[h->cs_id] = timestamp;
        }    

        if(ts == 0xFFFFFF)
        {
            memcpy(p,&timestamp,4);
            p += 4;
        }    

        BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_,p-out_current_);
        sending_bufs_.emplace_back(std::move(nheader));
        out_current_ = p;

        prev->cs_id = h->cs_id;
        prev->msg_len = h->msg_len;
        prev->msg_sid = h->msg_sid;
        prev->msg_type = h->msg_type;
        if(fmt == kRtmpFmt0)
        {
            prev->timestamp = timestamp;
        }
        else 
        {
            prev->timestamp += timestamp;
        }
        
        const char *body = packet->Data();
        int32_t bytes_parsed = 0;
        while(true)
        {
            const char * chunk = body+bytes_parsed;
            int32_t size = h->msg_len - bytes_parsed;
            size = std::min(size,out_chunk_size_);

            BufferNodePtr node = std::make_shared<BufferNode>((void*)chunk,size);
            sending_bufs_.emplace_back(std::move(node));
            bytes_parsed += size;

            if(bytes_parsed<h->msg_len)
            {
                if(out_current_ - out_buffer_ >= 4096)
                {
                    RTMP_ERROR << "rtmp had no enough out header buffer.";
                    break;
                }
                char *p = out_current_;

                if(h->cs_id<64)
                {
                    *p++ = (char)(0xC0|h->cs_id);
                }
                else if(h->cs_id<(64+256))
                {
                    *p++ = (char)(0xC0|0); 
                    *p++ = (char)(h->cs_id - 64);
                }
                else 
                {
                    *p++ = (char)(0xC0|1); 
                    uint16_t cs = h->cs_id-64;
                    memcpy(p,&cs,sizeof(uint16_t));
                    p += sizeof(uint16_t);
                }
                if(ts == 0xFFFFFF)
                {
                    memcpy(p,&timestamp,4);
                    p += 4;
                }      

                BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_,p-out_current_);
                sending_bufs_.emplace_back(std::move(nheader));
                out_current_ = p;      
            }
            else 
            {
                break;
            }
        }
        return true;
    }
    return false;
}

void RtmpContext::Send()
{

    if (sending_)
    {
        // RTMP_ERROR << "RtmpContext::Send() is sending_ now \n";
        return;
    }
    sending_ = true;

    for (int i = 0; i < 10; i++)
    {
        if (out_waiting_queue_.empty())
        {
            break;
        }
        PacketPtr packet = std::move(out_waiting_queue_.front());
        out_waiting_queue_.pop_front();
        // auto header = packet->Ext<RtmpMsgHeader>();

        // RTMP_TRACE << "send packer, type : " << static_cast<int>(header ->msg_type)
        //             << " csid :" << header -> cs_id
        //             << " stream id " << header->msg_sid
        //             ;

        auto ret = BuildChunk(std::move(packet));
        // if (!ret)
        // {
        //     RTMP_TRACE << "Build Chunk Error \n";
        // }
        // else
        // {
        //     RTMP_TRACE << "Build Chunk Success \n";
        // }
    }
    if (!sending_bufs_.empty())
    {   
        connection_->Send(sending_bufs_);
    }
    else
    {
        sending_ = false;
    }
}

bool RtmpContext::Ready() const
{
    return !sending_;
}
bool RtmpContext::BuildChunk (PacketPtr &&packet,uint32_t timestamp,bool fmt0)
{
    RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
    if(h)
    {
        RtmpMsgHeaderPtr &prev = out_message_headers_[h->cs_id];
        bool use_delta = !fmt0 && prev && timestamp >= prev->timestamp && h->msg_sid == prev->msg_sid;
        if(!prev)
        {
            prev = std::make_shared<RtmpMsgHeader>();
        }
        int fmt = kRtmpFmt0;
        if(use_delta)
        {
            fmt = kRtmpFmt1 ;
            timestamp -= prev->timestamp;
            if(h->msg_type == prev->msg_type
                && h->msg_len == prev->msg_len)
            {
                fmt = kRtmpFmt2;
                if(timestamp == out_deltas_[h->cs_id]) 
                {
                    fmt = kRtmpFmt3;
                }   
            }
        }

        char *p = out_current_;

        if(h->cs_id<64)
        {
            *p++ = (char)((fmt<<6)|h->cs_id);
        }
        else if(h->cs_id<(64+256))
        {
           *p++ = (char)((fmt<<6)|0); 
           *p++ = (char)(h->cs_id - 64);
        }
        else 
        {
           *p++ = (char)((fmt<<6)|1); 
           uint16_t cs = h->cs_id-64;
           memcpy(p,&cs,sizeof(uint16_t));
           p += sizeof(uint16_t);
        }

        auto ts = timestamp;
        if(timestamp >= 0xFFFFFF)
        {
            ts = 0xFFFFFF;
        }

        if(fmt == kRtmpFmt0)
        {
            p += BytesWriter::WriteUint24T(p,ts);
            p += BytesWriter::WriteUint24T(p,h->msg_len);
            p += BytesWriter::WriteUint8T(p,h->msg_type);

            memcpy(p,&h->msg_sid,4);
            p += 4;
            out_deltas_[h->cs_id] = 0;
        } 
        else if(fmt == kRtmpFmt1)
        {
            p += BytesWriter::WriteUint24T(p,ts);
            p += BytesWriter::WriteUint24T(p,h->msg_len);
            p += BytesWriter::WriteUint8T(p,h->msg_type);
            out_deltas_[h->cs_id] = timestamp;
        }
        else if(fmt == kRtmpFmt2)
        {
            p += BytesWriter::WriteUint24T(p,ts);
            out_deltas_[h->cs_id] = timestamp;
        }    

        if(ts == 0xFFFFFF)
        {
            memcpy(p,&timestamp,4);
            p += 4;
        }    

        BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_,p-out_current_);
        sending_bufs_.emplace_back(std::move(nheader));
        out_current_ = p;

        prev->cs_id = h->cs_id;
        prev->msg_len = h->msg_len;
        prev->msg_sid = h->msg_sid;
        prev->msg_type = h->msg_type;
        if(fmt == kRtmpFmt0)
        {
            prev->timestamp = timestamp;
        }
        else 
        {
            prev->timestamp += timestamp;
        }
        
        const char *body = packet->Data();
        int32_t bytes_parsed = 0;
        while(true)
        {
            const char * chunk = body+bytes_parsed;
            int32_t size = h->msg_len - bytes_parsed;
            size = std::min(size,out_chunk_size_);

            BufferNodePtr node = std::make_shared<BufferNode>((void*)chunk,size);
            sending_bufs_.emplace_back(std::move(node));
            bytes_parsed += size;

            if(bytes_parsed<h->msg_len)
            {
                if(out_current_ - out_buffer_ >= 4096)
                {
                    RTMP_ERROR << "rtmp had no enough out header buffer.";
                    break;
                }
                char *p = out_current_;

                if(h->cs_id<64)
                {
                    *p++ = (char)(0xC0|h->cs_id);
                }
                else if(h->cs_id<(64+256))
                {
                    *p++ = (char)(0xC0|0); 
                    *p++ = (char)(h->cs_id - 64);
                }
                else 
                {
                    *p++ = (char)(0xC0|1); 
                    uint16_t cs = h->cs_id-64;
                    memcpy(p,&cs,sizeof(uint16_t));
                    p += sizeof(uint16_t);
                }
                if(ts == 0xFFFFFF)
                {
                    memcpy(p,&timestamp,4);
                    p += 4;
                }      

                BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_,p-out_current_);
                sending_bufs_.emplace_back(std::move(nheader));
                out_current_ = p;      
            }
            else 
            {
                break;
            }
        }
        out_sending_packets_.emplace_back(std::move(packet));
        return true;
    }
    return false;
}

// bool RtmpContext::BuildChunk(PacketPtr &&packet, uint32_t timestamp, bool fmt0)
// {
//     // 获取头部
//     auto header = packet->Ext<RtmpMsgHeader>();

//     RTMP_TRACE << "BuildChunk send packer, type : " << static_cast<int>(header ->msg_type);

//     if (!header) 
//     {
//         RTMP_ERROR << "BuildChunk Header is Null !!! \n";
//         return false;
//     }

    
//     auto& pre = out_message_headers_[header -> cs_id];

//     // 是否是具有时间间隔， 
//     /**
//      * 1、 fmt0不具有  时间差
//      * 2、 有时间差， 必须有前一个节点，这样才能计算时间差
//      * 3、 时间戳必须必之前一个时间戳大
//      * 4、 得是同一个 数据流的
//      */
//     bool is_deltal = !fmt0 && pre && timestamp >= pre -> timestamp && header -> msg_sid == pre -> msg_sid;

//     if (!pre)
//     {
//         pre = std::make_shared<RtmpMsgHeader>();
//     }

//     // 先确定是fmt 0123
//     int fmt = kRtmpFmt0;
//     if (is_deltal)
//     {
//         fmt = kRtmpFmt1;
//         // 长度 和 类型相同
//         if (header -> msg_len == pre -> msg_len && header -> msg_type == pre -> msg_type)
//         {
//             fmt = kRtmpFmt2;
//             // deltas 相同
//             timestamp -= pre -> timestamp;
//             if (out_deltas_[header -> cs_id] == timestamp)
//             {
//                 fmt = kRtmpFmt3;
//             }
//         }
//     }

//     // 开始填充缓冲区的数据 
//     char *p = out_current_;

//     // 填入 fmt 和 csid
//     if (header -> cs_id < 64)
//     {
//         *p++ = (char)((fmt << 6) | header -> cs_id);
//     }
//     else if (header -> cs_id < 64 + 256)
//     {
//         *p++ = (char)((fmt << 6) | 0);
//         *p++ = (char)(header -> cs_id - 64);
//     }
//     else
//     {
//         *p++ = (char)((fmt << 6) | 1);
//         uint16_t temp = header -> cs_id - 64;
//         memcpy(p, &temp, sizeof(uint16_t));
//         p += sizeof(uint16_t);
//     }


//     // 此时ts已经是上个与上一个chunk的timestamp的差值
//     auto ts = timestamp;
//     if (timestamp >= 0xffffff)
//     {
//         ts = 0xffffff;
//     }

//     // 按不同的fmt 开始填写数据
//     if (fmt == kRtmpFmt0)
//     {
//         p += BytesWriter::WriteUint24T(p, ts);
//         p += BytesWriter::WriteUint24T(p, header -> msg_len);
//         p += BytesWriter::WriteUint8T(p, header -> msg_type);
//         RTMP_TRACE << "fmt 0 send packer, type : " << (static_cast<int>(*(uint8_t *)(p - 1)));
//         memcpy(p, &header -> msg_sid, 4);
//         p += 4;
//         out_deltas_[header -> cs_id] = 0;
//     }
//     else if (fmt == kRtmpFmt1)
//     {
//         p += BytesWriter::WriteUint24T(p, ts);
//         p += BytesWriter::WriteUint24T(p, header -> msg_len);
//         p += BytesWriter::WriteUint8T(p, header -> msg_type);
//         RTMP_TRACE << "fmt 0 send packer, type : " << (static_cast<int>(*(uint8_t *)(p - 1)));


//         out_deltas_[header -> cs_id] = timestamp;
//     }
//     else if (fmt == kRtmpFmt2)
//     {
//         p += BytesWriter::WriteUint24T(p, ts);
//         out_deltas_[header -> cs_id] = timestamp;
//     }
//     else if (fmt == kRtmpFmt3)
//     {
        
//     }

//     // 如果有time delta ，则进行填写
//     if (timestamp >= 0xffffff)
//     {
//         memcpy(p, &timestamp, 4);
//         p += 4;
//     }

//     // 先发送头部数据
//     auto nheader = std::make_shared<BufferNode>(out_buffer_, p - out_current_);
//     sending_bufs_.push_back(std::move(nheader));
//     out_current_ = p;

//     // 更新前一个信息
//     pre -> cs_id = header -> cs_id;
//     pre -> msg_len = header -> msg_len;
//     pre -> msg_sid = header -> msg_sid;
//     pre -> msg_type = header -> msg_type;

//     if (fmt == kRtmpFmt0)
//     {
//         pre -> timestamp = timestamp;
//     }
//     else
//     {
//         pre -> timestamp += timestamp;
//     }

//     // 准备发送消息体
//     const char *body = packet -> Data();

//     int32_t bytes_parsed = 0;

//     while (true)
//     {
//         // 此次数据写入的地址
//         const char * dest = body + bytes_parsed;

//         // 此次数据写入的长度
//         int32_t size = header -> msg_len - bytes_parsed;

//         // 每次写的最大数据不能超过out_chunk_size_
//         size = std::min(size, out_chunk_size_);

//         auto node = std::make_shared<BufferNode>((void *)dest, size);

//         // 将数据放置到发送队列
//         sending_bufs_.push_back(std::move(node));

//         bytes_parsed += size;

//         if (bytes_parsed >= header -> msg_len)
//         {
//             break;
//         }

//         if (out_current_ - out_buffer_ >= MESSAGE_CHUNK_SIZE)
//         {
//             RTMP_ERROR << "rtmp had no enough buff : out_current_ :" << out_current_ << " out_buffer_ " << out_buffer_;
//             break;
//         }
//         char *p = out_current_;

//         // 发送一个头部 fmt3
//         // 填入 fmt 和 csid
//         if (header -> cs_id < 64)
//         {
//             *p++ = (char)(0xc0 | header -> cs_id);
//         }
//         else if (header -> cs_id < 64 + 256)
//         {
//             *p++ = (char)(0xc0 | 0);
//             *p++ = (char)(header -> cs_id - 64);
//         }
//         else
//         {
//             *p++ = (char)(0xc0 | 1);
//             uint16_t temp = header -> cs_id - 64;
//             memcpy(p, &temp, sizeof(uint16_t));
//             p += sizeof(uint16_t);
//         }
        
//         if (timestamp >= 0xffffff)
//         {
//             memcpy(p, &timestamp, 4);
//             p += 4;
//         }
        
//         auto nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
//         // 增加生命周期
//         sending_bufs_.push_back(std::move(nheader));
//         out_current_ = p;
//     }

//     out_sending_packets_.push_back(std::move(packet));

//     return true;
// }

void RtmpContext::CheckAfterSend()
{

    sending_ = false;
    out_current_ = out_buffer_;
    sending_bufs_.clear();
    out_sending_packets_.clear();

    if (!out_waiting_queue_.empty())
    {
        RTMP_TRACE << " still has waiting send \n";
        Send();
    }
    else
    {
        if (rtmp_callback_)
        {
           rtmp_callback_->OnActive(connection_);
        }
    }
}

void RtmpContext::PushOutQueue(PacketPtr &&packet)
{
    out_waiting_queue_.push_back(std::move(packet));
    Send();
}


void RtmpContext::SendSetChunkSize()
{
    auto packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    packet->SetExt(header);
    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 4;
        header -> timestamp = 0;    
        header -> msg_type = kRtmpMsgTypeChunkSize;
    }

    auto p = packet->Data();
    packet -> SetPacketSize(BytesWriter::WriteUint32T(p, out_chunk_size_));
    RTMP_TRACE << " send chunk size : " << out_chunk_size_ << " to host " << connection_ -> PeerAddr().ToIpPort() ;
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendAckWindowSize()
{
    auto packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    packet->SetExt(header);
    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> timestamp = 0;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 4;
        header -> msg_type = kRtmpMsgTypeWindowACKSize;
    }

    auto p = packet->Data();
    packet -> SetPacketSize(BytesWriter::WriteUint32T(p, ack_size_));
    RTMP_TRACE << " send ack size : " << ack_size_ << " to host" << connection_ -> PeerAddr().ToIpPort() ;
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendSetPeerBandwidth()
{
    auto packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();


    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 5;
        header -> timestamp = 0;
        header -> msg_type = kRtmpMsgTypeSetPeerBW;
        packet->SetExt(header);
    }

    auto p = packet->Data();
    p += BytesWriter::WriteUint32T(p, ack_size_);
    *p++ = 0x02;
    packet -> SetPacketSize(5);
    RTMP_TRACE << " send band width : " << ack_size_ << " to host" << connection_ -> PeerAddr().ToIpPort() ;
    PushOutQueue(std::move(packet));
}

void RtmpContext::SentBytesRecv()
{
    // 超过窗口大小
    if (in_bytes_ >= ack_size_)
    {
        auto packet = Packet::NewPacket(64);
        RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

        packet->SetExt(header);
        if (header)
        {
            header -> cs_id = kRtmpCSIDCommand;
            header -> msg_sid = kRtmpMsID0;
            header -> msg_len = 4;
            header -> msg_type = kRtmpMsgTypeBytesRead;
            header -> timestamp = 0;
           packet->SetExt(header);

        }

        auto p = packet->Data();
        packet -> SetPacketSize(BytesWriter::WriteUint32T(p, in_bytes_));
        RTMP_TRACE << " send ack size : " << ack_size_ << " to host" << connection_ -> PeerAddr().ToIpPort();
        PushOutQueue(std::move(packet));
        in_bytes_ = 0;
    }
    
}

void RtmpContext::SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2)
{
    auto packet = Packet::NewPacket(64);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    if (header)
    {
        header -> cs_id = kRtmpCSIDCommand;
        header -> timestamp = 0;
        header -> msg_sid = kRtmpMsID0;
        header -> msg_len = 0;
        header -> msg_type = kRtmpMsgTypeUserControl;
        packet->SetExt(header);
    }

    char* body = packet->Data();
    char* p = body;
    p += BytesWriter::WriteUint16T(body, nType);
    p += BytesWriter::WriteUint32T(body, value1);
    if (nType == kRtmpEventTypeSetBufferLength)
    {
        p += BytesWriter::WriteUint32T(body, value2);
    }
    header -> msg_len = p - body;

    packet -> SetPacketSize(header -> msg_len);
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
        RTMP_TRACE << " handle chunk size errror, packet size : " << packet -> PacketSize() << connection_->PeerAddr().ToIpPort() ;
        return; 
    }

    auto size = BytesReader::ReadUint32T(packet->Data());

    RTMP_TRACE << " in chunk size change from : " << in_chunk_size_ << " to " << size  ;

    in_chunk_size_ = size;
    
}

void RtmpContext::HandleAckWindowSize(PacketPtr &packet)
{
    if (packet -> PacketSize() < 4)
    { 
        RTMP_ERROR << " handle  ack error, packet size : " << packet -> PacketSize() << connection_->PeerAddr().ToIpPort() ;
        return; 
    }

    auto size = BytesReader::ReadUint32T(packet->Data());

    RTMP_TRACE << " in ack size change from : " << ack_size_ << " to " << size ;

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
            RTMP_TRACE << "recv stream dry value" << value << " host:" << connection_->PeerAddr().ToIpPort() ;
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
            RTMP_TRACE << "recv ping request value" << value << " host:" << connection_->PeerAddr().ToIpPort() ;
            SendUserCtrlMessage(kRtmpEventTypePingResponse,value,0);
            break;
        }   
        case kRtmpEventTypePingResponse:
        {
            RTMP_TRACE << "recv ping response value" << value << " host:" << connection_->PeerAddr().ToIpPort() ;
            break;
        }
        default:
            break;
    }
}

void RtmpContext::HandleAMFMessage(PacketPtr &packet, bool amf3)
{


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
        RTMP_ERROR << "amf message decode error" ;
        return;
    }
    const std::string &method = obj.Property(0)->String();
    RTMP_TRACE << " -------handler AMF message : " << connection_ -> PeerAddr().ToIpPort() << " ------";


    RTMP_TRACE << "--------AMF command: " << method << " host:" << connection_->PeerAddr().ToIpPort() << " --------- ";

    auto iter = commands_.find(method);
    if(iter == commands_.end())
    {
        RTMP_TRACE << " not surpport method: " << method << " host:" << connection_->PeerAddr().ToIpPort() << " --------- ";
        return ;
    }
    iter->second(obj);

    RTMP_TRACE << "--------AMF command:" << method << " host:" << connection_->PeerAddr().ToIpPort() << " ----end----- ";


}


void RtmpContext::SendConnect()
{
    SendSetChunkSize();
    PacketPtr packet = Packet::NewPacket(1024);
    auto header = std::make_shared<RtmpMsgHeader>();

    header -> msg_type = kRtmpMsgTypeAMFMessage;
    header -> cs_id = kRtmpCSIDAMFIni;
    header -> msg_len = 0;
    header -> msg_sid = 0;

    packet -> SetExt(header);

    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "connect");
    p += AMFAny::EncodeNumber(p, 1.0);
    *p++ = kAMFObject;
    p += AMFAny::EncodeNamedString(p, "app", app_);
    p += AMFAny::EncodeNamedString(p, "tcUrl", tc_url_);
    p += AMFAny::EncodeNamedBoolean(p, "fpad", false);
    p += AMFAny::EncodeNamedNumber(p, "capabilities", 31.0);
    p += AMFAny::EncodeNamedNumber(p, "audioCodecs", 1639.0);
    p += AMFAny::EncodeNamedNumber(p, "videoCodecs", 252.0);
    p += AMFAny::EncodeNamedNumber(p, "videoFunction", 1.0);
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;

    header -> msg_len = p - body;
    packet -> SetPacketSize(header -> msg_len);
    RTMP_TRACE << "send connect msg_len:" << header->msg_len << " to host:" << connection_->PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}


void RtmpContext::HandleConnect(AMFObject &obj)
{
    bool amf3 = false;
    // 推流地址一定是有的
    tc_url_ = obj.Property("tcUrl") -> String();

    AMFObjectPtr sub_obj = obj.Property(2)->Object();
    if(sub_obj)
    {
        app_ = sub_obj->Property("app")->String();
        if(sub_obj->Property("objectEncoding"))
        {
            amf3 = sub_obj->Property("objectEncoding")->Number() == 3.0;
        }
    }

    // app_ = obj.Property("app")->String();
    // amf3 = obj.Property("objectEncoding")->Number() == 3.0;
    
    RTMP_TRACE << "----responce connect tcUrl:" << tc_url_ << " app: " << app_ << " amf3:"  << amf3 << " -----";

    SendAckWindowSize();

    SendSetPeerBandwidth();

    SendSetChunkSize();


    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_sid = 0;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage; 

    packet->SetExt(header);



    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "_result");
    p += AMFAny::EncodeNumber(p, 1.0);
    *p++ = kAMFObject;
    p += AMFAny::EncodeNamedString(p, "fmsVer", "FMS/3,0,1,123");
    p += AMFAny::EncodeNamedNumber(p, "capabilities", 31);
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;
    *p++ = kAMFObject; 
    p += AMFAny::EncodeNamedString(p, "level", "status");
    p += AMFAny::EncodeNamedString(p, "code", "NetConnection.Connect.Success");
    p += AMFAny::EncodeNamedString(p, "description", "Connection succeeded.");
    p += AMFAny::EncodeNamedNumber(p, "objectEncoding", amf3 ? 3.0 : 0);
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    PushOutQueue(std::move(packet));
    RTMP_TRACE << "----connect responce rtmp connec msg_len:" << header->msg_len << " from host:" << connection_->PeerAddr().ToIpPort() << " -------";


}


void RtmpContext::SendCreateStream()
{
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_sid = 0;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage; 
    packet->SetExt(header);

    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "createStream");
    p += AMFAny::EncodeNumber(p, 4.0);
    *p++ = kAMFNull;

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send create stream msg_len:" << header->msg_len << " to host:" << connection_->PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::HandleCreateStream(AMFObject &obj)
{
    RTMP_TRACE << " RtmpContext::HandleCreateStream(AMFObject &obj) "  ;
    auto tran_id = obj.Property(1)->Number();

    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_sid = 0;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage; 
    packet->SetExt(header);

    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "_result");
    p += AMFAny::EncodeNumber(p, tran_id);
    *p++ = kAMFNull;
    
    p += AMFAny::EncodeNumber(p, kRtmpMsID1);

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "create stream result msg_len:" << header->msg_len << " to host:" << connection_->PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendStatus(const std::string &level, const std::string &code, const std::string &description)
{
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_sid = 1;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage; 
    packet->SetExt(header);

    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "onStatus");
    p += AMFAny::EncodeNumber(p, 0);
    *p++ = kAMFNull;
    *p++ = kAMFObject;
    p += AMFAny::EncodeNamedString(p, "level", level);
    p += AMFAny::EncodeNamedString(p, "code", code);
    p += AMFAny::EncodeNamedString(p, "description", description);
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x09;


    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send status level:" << level << " code:" << code << " desc:" << description << " to host:" << connection_->PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendPlay()
{
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_sid = 1;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage; 
    packet->SetExt(header);

    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "play");
    p += AMFAny::EncodeNumber(p, 0);
    *p++ = kAMFNull;
    // 流名
    p += AMFAny::EncodeString(p, name_);
    // 播放的起始时间
    p += AMFAny::EncodeNumber(p, -1000.0);

    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send play name:"<< name_ 
            << " msg_len:" << header->msg_len 
            << " to host:" << connection_->PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}

void RtmpContext::HandlePlay(AMFObject &obj)
{
    auto tran_id = obj.Property(1)->Number();
    name_ = obj.Property(3)->String();
    ParseNameAndTcUrl();

    RTMP_TRACE << "recv play session_name: " << session_name_ 
            << " param: " << param_ 
            << " host: " << connection_->PeerAddr().ToIpPort();
    is_player_ = true;
    SendUserCtrlMessage(kRtmpEventTypeStreamBegin,1,0);
    SendStatus("status","NetStream.Play.Start","Start Playing");

    if(rtmp_callback_)
    {
        rtmp_callback_->OnPlay(connection_,session_name_,param_);
    }
}

void RtmpContext::ParseNameAndTcUrl()
{
    auto pos = app_.find_first_of("/");
    if(pos!=std::string::npos)
    {
        app_ = app_.substr(pos+1);
    }

    param_.clear();
    pos = name_.find_first_of("?");
    if(pos != std::string::npos)
    {
        param_ = name_.substr(pos+1);
        name_ = name_.substr(0,pos);
    }

    std::string domain;

    std::vector<std::string> list = base::StringUtils::SplitString(tc_url_,"/");
    // if(list.size()==5)//rmtp://ip/domain:port/app
    // {
    //     domain = list[3];
    //     app_ = list[4];
    // }
    // else if(list.size() == 4) //rmtp://domain:port/app
    // {
    //     domain = list[2];
    //     app_ = list[3];
    // }
    if(list.size() == 3) //rmtp://domain:port/app
    {
        domain = list[1];
        app_ = list[2];
    }

    // 去掉端口号
    auto p = domain.find_first_of(":");
    if(p!=std::string::npos)
    {
        domain = domain.substr(0,p);
    }

    session_name_.clear();
    session_name_ += domain;
    session_name_ += "/";
    session_name_ += app_;
    session_name_ += "/";
    session_name_ += name_;

    RTMP_TRACE << " session_name: " << session_name_  
            << " param: " << param_ 
            << " host: " << connection_->PeerAddr().ToIpPort();
}

void RtmpContext::HandleError(AMFObject &obj)
{
    const std::string &description = obj.Property(3)->Object()->Property("description")->String();
    RTMP_ERROR << "recv error description:" << description << " host:" << connection_->PeerAddr().ToIpPort();
    connection_->ForceClose();
}

void RtmpContext::HandleResult(AMFObject &obj)
{
    auto id = obj.Property(1)->Number();
    RTMP_TRACE << "recv result id:" << id << " host:" << connection_->PeerAddr().ToIpPort();
    if(id == 1)
    {
        SendCreateStream();
    }
    else if(id == 4)
    {
        if(is_player_)
        {
            SendPlay();
        }
        else 
        {
            SendPublish();
        }
    }
    else if(id == 5)
    {
        if(rtmp_callback_)
        {
            rtmp_callback_->OnPublishPrepare(connection_);
        }
    }

}
void RtmpContext::SendPublish()
{
    PacketPtr packet = Packet::NewPacket(1024);
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
    header->cs_id = kRtmpCSIDAMFIni;
    header->msg_sid = 1;
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeAMFMessage; 
    packet->SetExt(header);

    char *body = packet->Data();
    char *p = body;

    p += AMFAny::EncodeString(p, "publish");
    p += AMFAny::EncodeNumber(p, 5);
    *p++ = kAMFNull;
    p += AMFAny::EncodeString(p, name_);
    p += AMFAny::EncodeString(p, "live");


    header->msg_len = p - body;
    packet->SetPacketSize(header->msg_len);
    RTMP_TRACE << "send publish name:"<< name_ 
            << " msg_len:" << header->msg_len 
            << " to host:" << connection_->PeerAddr().ToIpPort();
    PushOutQueue(std::move(packet));
}
void RtmpContext::HandlePublish(AMFObject &obj)
{
    auto tran_id = obj.Property(1)->Number();
    name_ = obj.Property(3)->String();
    ParseNameAndTcUrl();

    RTMP_TRACE << "recv publish session_name: " << session_name_ 
            << " param: " << param_ 
            << " host: " << connection_->PeerAddr().ToIpPort();
    is_player_ = false;
    SendStatus("status","NetStream.Publish.Start","Start Publishing");

    if(rtmp_callback_)
    {
        rtmp_callback_->OnPublish(connection_,session_name_,param_);
    }
}

void RtmpContext::Play(const std::string &url)
{
    is_client_ = true;
    is_player_ = true;
    tc_url_ = url;
    ParseNameAndTcUrl();
}
void RtmpContext::Publish(const std::string &url) 
{
    is_client_ = true;
    is_player_ = false;
    tc_url_ = url;
    ParseNameAndTcUrl();
}

void RtmpContext::SetPacketType(PacketPtr &packet)
{
    if(packet->PacketType() == kRtmpMsgTypeAudio)
    {
        packet->SetPacketType(kPacketTypeAudio);
    }
    else if(packet->PacketType() == kRtmpMsgTypeVideo)
    {
        packet->SetPacketType(kPacketTypeVideo);
    } 
    else if(packet->PacketType() == kRtmpMsgTypeAMFMeta)
    {
        packet->SetPacketType(kPacketTypeMeta);
    }     
    else if(packet->PacketType() == kRtmpMsgTypeAMF3Meta)
    {
        packet->SetPacketType(kPacketTypeMeta3);
    }              
}