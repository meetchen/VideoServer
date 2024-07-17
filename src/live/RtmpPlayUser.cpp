/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-15 14:18:00
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 18:54:40
 * @FilePath: /VideoServer/src/live/RtmpPlayUser.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${995122760@qq.com}, All Rights Reserved. 
 */
#include "live/RtmpPlayUser.h"
#include "live/Stream.h"
#include "mmedia/base/MMediaLog.h"
#include "base/TTime.h"
#include "mmedia/rtmp/RtmpContext.h"
#include "live/base/TimeCorrector.h"

using namespace vdse::live;


RtmpPlayUser::RtmpPlayUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s)
:PlayerUser(ptr, stream, s)
{

}

bool RtmpPlayUser::PostFrames()
{
    RTMP_DEBUG << " meta " << meta_ << " audio_header " << audio_header_ << " video header " << video_header_ << " out size :" << out_frames_.size();
    if (!stream_->Ready() || !stream_->HasMedia())
    {
        RTMP_DEBUG << " stream Ready :" << stream_->Ready() << " stream has media : " << stream_->HasMedia();
        return false;
    }

    stream_->GetFrames(std::dynamic_pointer_cast<PlayerUser>(shared_from_this()));

    if (meta_)
    {
        auto ret = PushFrame(meta_, true);
        if (ret)
        {
            RTMP_INFO << " rtmp sent meta now : " << base::TTime::NowMS() << " host : " << user_id_;
            meta_.reset();
        }
        else
        {
            RTMP_DEBUG << " PushFrame meta failed  host : " << user_id_;
            return false;
        }

    }
    else if (audio_header_)
    {
        auto ret = PushFrame(audio_header_, true);
        if (ret)
        {
            RTMP_INFO << " rtmp sent audio_header_ now : " << base::TTime::NowMS() << " host : " << user_id_;
            audio_header_.reset();
        }
        else
        {
            RTMP_DEBUG << " PushFrame audio_header_ failed  host : " << user_id_;
            return false;
        }
    }
    else if (video_header_)
    {
        auto ret = PushFrame(video_header_, true);
        if (ret)
        {
            RTMP_INFO << " rtmp sent video_header_ now : " << vdse::base::TTime::NowMS() << " host : " << user_id_;
            video_header_.reset();
        }
        else
        {
            RTMP_DEBUG << " PushFrame video_header_ failed  host : " << user_id_;
            return false;
        }
    }
    else if (!out_frames_.empty())
    {
        auto ret = PushFrames(out_frames_);
        if (ret)
        {
            out_frames_.clear();
        }
        else
        {
            RTMP_DEBUG << " PushFrame out_frames_ failed  host : " << user_id_;
            return false;
        }
    }
    else
    {
        Deactive();
    }

    return true;
}

UserType RtmpPlayUser::GetUserType() const
{
    return UserType::kUserTypePlayerRtmp;
}


bool RtmpPlayUser::PushFrame(PacketPtr & packet, bool is_header)
{
    auto cx = connection_->GetContext<RtmpContext>(kRtmpContext);
    if (!cx || !cx->Ready())
    {
        if (!cx)
        {
            RTMP_ERROR << " don't have RtmpContext!!!!";
        }
        else
        {
            RTMP_ERROR << " have RtmpContext!!!! " << " cx-ready : " << cx->Ready();
        }
        return false;
    }
    auto ts = 0;
    if (!is_header)
    {
        ts = time_corrector_.CorrectTimestamp(packet);
    }
    auto ret = cx->BuildChunk(packet, ts, is_header);
    if (!ret) 
    {
        RTMP_DEBUG << "build chunk failed";
        return false;
    }
    cx->Send();
    return true;
}

bool RtmpPlayUser::PushFrames(std::vector<PacketPtr> & packets)
{
    auto cx = connection_->GetContext<RtmpContext>(kRtmpContext);
    if (!cx || !cx->Ready())
    {
        if (!cx)
        {
            RTMP_ERROR << " don't have RtmpContext";
        }
        else
        {
            RTMP_ERROR << " have RtmpContext!!!! " << " cx-ready : " << cx->Ready();
        }
        return false;
    }
    for (auto &packet : packets)
    {
        auto ts = time_corrector_.CorrectTimestamp(packet);
        auto ret = cx->BuildChunk(packet, ts, false);
        if (!ret) 
        {
            return false;
        }
    }
    cx->Send();
    return true;
}