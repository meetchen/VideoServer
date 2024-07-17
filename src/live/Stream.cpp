/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 18:11:05
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 18:18:53
 * @FilePath: /VideoServer/src/live/Stream.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "live/Stream.h"
#include "base/TTime.h"
#include "live/base/CodecUtils.h" 
#include "live/base/LiveLog.h"
#include "live/Session.h"

using namespace vdse::live;


Stream::Stream(Session &s,const std::string &session_name)
:session_(s), session_name_(session_name), packet_buffer_(packet_buffer_size_)
{
    start_timestamp_ = vdse::base::TTime::NowMS();
    stream_time_ = vdse::base::TTime::NowMS();
}

int64_t Stream::ReadyTime() const 
{
    return ready_time_;
}
int64_t Stream::SinceStart() const 
{
    return vdse::base::TTime::NowMS() - start_timestamp_;
}
bool Stream::Timeout()
{
    if (vdse::base::TTime::NowMS() - stream_time_ > 20 * 1000)
    {
        LIVE_TRACE << "Stream Timeout, stream_time_: " << stream_time_;
        return true;
    }
    return false;
}
int64_t Stream::DataTime() const 
{
    return data_coming_time_;
}
const std::string &Stream::SessionName() const 
{
    return session_name_;
}
int32_t Stream::StreamVersion() const
{
    return stream_version_;
}
bool Stream::HasMedia() const
{
    return has_audio_ || has_video_ || has_meta_;
}
bool Stream::Ready() const
{
    return ready_;
}

void Stream::AddPacket(PacketPtr && packet)
{   
    auto t = time_corrector_.CorrectTimestamp(packet);
    packet->SetTimeStamp(t);

    {
        std::lock_guard<std::mutex> lk(lock_);

        auto index = ++frame_index_;

        packet -> SetIndex(index);

        if (packet->IsVideo() && CodecUtils::IsKeyFrame(packet))
        {
            LIVE_TRACE << " get a video and key frame packet, index = " << index;
            packet->SetPacketType(kPacketTypeVideo | kFrameTypeKeyFrame);
            SetReady(true);
        }

        if (CodecUtils::IsCodecHeader(packet))
        {
            codec_headers_.ParseCodecHeader(packet);
            if (packet -> IsVideo())
            {
                LIVE_TRACE << " get a video frame packet";
                has_video_ = true;
                stream_version_++;
            }
            else if (packet -> IsAudio())
            {
                LIVE_TRACE << " get a audio frame packet";
                has_audio_ = true;
                stream_version_ ++;
            }
            else if (packet -> IsMeta())
            {
                LIVE_TRACE << " get a meta frame packet";
                has_meta_ = true;
                stream_version_++;
            }
        }

        gop_mgr_.AddFrame(packet);
        
        if (index % 500 ==  0)
        {
           LIVE_TRACE << " add packets , index = " << index;
        }

        packet_buffer_[index % packet_buffer_size_] = std::move(packet);
        
        // 当前记录的帧的序号 减去最多能保存的帧数个数，记为当前保存的最小帧
        auto min_idx = frame_index_ - packet_buffer_size_;

        if (min_idx > 0)
        {
            gop_mgr_.ClearExpriedGop(min_idx);
        }
    }

    if (data_coming_time_ == 0)
    {
        data_coming_time_ = vdse::base::TTime::NowMS();
    }

    stream_time_ = vdse::base::TTime::NowMS();

    auto frame = frame_index_.load();
    if (frame < 300 || frame % 5 == 0)
    {
        session_.ActiveAllPlayers();
    }
}

void Stream::GetFrames(const PlayerUserPtr &user)
{
    // 是否有收到媒体信息
    if(!HasMedia())
    {
        return;
    }
    // user是否有数据在发
    if (user->video_header_ || user->audio_header_ || user->meta_ || !user->out_frames_.empty())
    {
        return;
    }

    // 检测是否发送过头部了

    if (user->out_index_ >= 0) // 如果发送过数据帧了 那么肯定发送过头部了
    {
        // 检测是否需要跳帧
        auto min_idx = frame_index_ - packet_buffer_size_;

        int contency_latency = user->GetAppInfo()->content_latency;

        // 表示已经发送的数据 已经落后 与缓冲区内的数据了 需要跳帧
        // 如果当前收到的最后的时间戳，比正在发送的实际戳，大于两倍的延时，则需要跳帧
        if (user->out_index_ < min_idx
            || (
                (gop_mgr_.LastestTimeStamp() - user->out_frame_timestamp_) > 2 * contency_latency
            )
        )
        {
             LIVE_INFO << "need skip out index:" << user->out_index_
                    << ",min idx:" << min_idx
                    << ",out timestamp:" << user->out_frame_timestamp_
                    << ",latest timestamp:" << gop_mgr_.LastestTimeStamp();
            SkipFrame(user);
        }
    }
    else
    {
        // 定位GOP
        if (!LocateGop(user))
        {
            // LIVE_DEBUG << " local gop Failed";
            return;
        }
    }

    GetNextFrame(user);


}
bool Stream::LocateGop(const PlayerUserPtr &user)
{
    // 通过延迟 获取最接近的GGOP
    auto content_lantency = user->GetAppInfo()->content_latency;
    // 保存所获取的GOP，的延迟
    int lantency = 0;
    auto idx = gop_mgr_.GetGopByLatency(content_lantency, lantency);
    if (idx != -1)
    {
        user->out_index_ = idx - 1;
    }
    else
    {
        auto elapsed = user->ElapsedTime();
        // 如果耗时超过1秒 并且还未触发过超时
        if (elapsed > 1000 && !user->wait_timeout_)
        {
            LIVE_DEBUG << "wait Gop keyFrame timeout, host " << user->user_id_ << " elapsed : "<<elapsed;
            user->wait_timeout_ = true;
        }
        return false;
    }

    // 是否需要发meta
    user->wait_meta_ = (user->wait_meta_ && has_meta_);

    if (user->wait_meta_)
    {
        // 找到距离该idx最近的一个meta
        auto meta = codec_headers_.Meta(idx);
        LIVE_DEBUG << " need meta, try to find mete";
        if(meta)
        {
            user->wait_meta_ = false;
            user->meta_ = meta;
            user->meta_index_ = meta->Index();
            LIVE_DEBUG << " need meta and meta in : " << meta->Index();
        }
    }
    

    user->wait_audio_ = (user->wait_audio_ && has_audio_);

    if (user->wait_audio_)
    {
        // 找到距离该idx最近的一个meta
        auto audio = codec_headers_.AudioHeader(idx);
        LIVE_DEBUG << " need audio, try to find audio";

        if(audio)
        {
            user->wait_audio_ = false;
            user->audio_header_ = audio;
            user->audio_header_index_ = audio->Index();
            LIVE_DEBUG << " need audio and audio in : " << audio->Index();

        }
    }


    user->wait_video_ = (user->wait_video_ && has_video_);

    if (user->wait_video_)
    {
        // 找到距离该idx最近的一个meta
        auto video = codec_headers_.VideoHeader(idx);
        LIVE_DEBUG << " need video, try to find video";

        if(video)
        {
            user->wait_video_ = false;
            user->video_header_ = video;
            user->video_header_index_ = video->Index();
            LIVE_DEBUG << " need video and video in : " << video->Index();
        }
    }

    if (user->wait_audio_ || user->wait_meta_ || user->wait_video_)
    {
        auto elapsed = user->ElapsedTime();
        // 如果耗时超过1秒 并且还未触发过超时
        if (elapsed > 1000 && !user->wait_timeout_)
        {
            LIVE_DEBUG << "wait Gop keyframe timeout elapsed:" << elapsed 
                    << "ms,frame index:" << frame_index_
                    << ",gop size:" << gop_mgr_.GopSize()
                    <<". host:" << user->user_id_;
            user->wait_timeout_ = true;
        }
        return false;
    }

    user->wait_audio_ = true;
    user->wait_meta_ = true;
    user->wait_video_ = true;
    user->out_version_ = stream_version_;

    auto elapsed = user->ElapsedTime();

    LIVE_DEBUG << "locate GOP sucess.elapsed:" << elapsed 
                << "ms,gop idx:" << idx
                << ",frame index:" << frame_index_
                << ",lantency:" << lantency
                << ",user:" << user->user_id_;
    return true;

}
void Stream::SkipFrame(const PlayerUserPtr &user)
{
    int content_lantency = user->GetAppInfo()->content_latency;
    int lantency = 0;
    auto idx = gop_mgr_.GetGopByLatency(content_lantency,lantency);

    // 如果跳帧比当前发送的index 还小，就是已经被发送， 不能说跳到之前了
    if(idx == -1 || idx <= user->out_index_)
    {
        return;
    }

    auto meta = codec_headers_.Meta(idx);
    if(meta)
    {   
        // 如果meta包 比之前发的meta包新
        if(meta->Index() > user->meta_index_)
        {
            user->meta_ = meta;
            user->meta_index_ = meta->Index();
        }
    }
    auto audio_header = codec_headers_.AudioHeader(idx);
    if(audio_header)
    {
        if(audio_header->Index()>user->audio_header_index_)
        {
            user->audio_header_ = audio_header;
            user->audio_header_index_ = audio_header->Index();
        }
    }
    auto video_header = codec_headers_.VideoHeader(idx);
    if(video_header)
    {
        if(video_header->Index()>user->video_header_index_)
        {
            user->video_header_ = video_header;
            user->video_header_index_ = video_header->Index();
        }
    }      

    LIVE_DEBUG << "skip frame " << user->out_index_ << "->" << idx
                << ",lantency:" << lantency 
                << ",frame_index:" << frame_index_
                << ",host:" << user->user_id_;
    user->out_index_ = idx - 1;  
}
void Stream::GetNextFrame(const PlayerUserPtr &user)
{
    auto idx = user->out_index_ + 1;
    auto max_idx = frame_index_.load();
    
    LIVE_TRACE << "get next frame, idx = " << idx 
                << " max_idx = " << max_idx
                << " out_frames_ has " << user->out_frames_.size();

    for(int i = 0;i < 10;i++)
    {
        if(idx>max_idx)
        {
            break;
        }
        auto &pkt = packet_buffer_[idx%packet_buffer_size_];
        if(pkt)
        {
            user->out_frames_.emplace_back(pkt);
            user->out_index_ = pkt->Index();
            user->out_frame_timestamp_ = pkt->TimeStamp();
            idx = pkt->Index() + 1;
        }
        else 
        {
            break;
        }
    }
    LIVE_TRACE << "get next frame, idx = " << idx 
                << " max_idx = " << max_idx
                << " out_frames_ has " << user->out_frames_.size();
}

void Stream::SetReady(bool ready)
{
    ready_ = ready;
    ready_time_ = vdse::base::TTime::NowMS();
}

