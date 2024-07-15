/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 17:13:11
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 17:15:08
 * @FilePath: /VideoServer/src/live/PlayerUser.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "live/PlayerUser.h"
#include "live/Stream.h"


using namespace vdse::live;

// PlayerUser::PlayerUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s)
// :User(ptr, stream, s)
// {

// }

PacketPtr PlayerUser::Meta() const
{
    return meta_;
}

PacketPtr PlayerUser::VideoHeader() const
{
    return video_header_;
}
PacketPtr PlayerUser::AudioHeader() const
{
    return audio_header_;
}
void PlayerUser::ClearMeta()
{
    meta_.reset();
}
void PlayerUser::ClearAudioHeader()
{
    audio_header_.reset();
}
void PlayerUser::ClearVideoHeader()
{
    video_header_.reset();
}

TimeCorrector& PlayerUser::GetTimeCorrector()
{
    return time_corrector_;
}
