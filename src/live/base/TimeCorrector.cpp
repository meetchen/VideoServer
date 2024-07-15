/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-09 15:11:27
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 15:38:59
 * @FilePath: /VideoServer/src/live/base/TimeCorrector.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "live/base/TimeCorrector.h"
#include "live/base/CodecUtils.h"


using namespace vdse::live;

/**
 * @description: 检查是否编码头部，第一个包
 * @param {PacketPtr} &packet
 * @return {*}
 */
uint32_t TimeCorrector::CorrectTimestamp(const PacketPtr &packet)
{
    if(!CodecUtils::IsCodecHeader(packet))
    {
        int32_t pt = packet->PacketType();
        if(pt == kPacketTypeVideo)
        {
            return CorrectVideoTimeStampByVideo(packet);
        }
        else if(pt == kPacketTypeAudio)
        {
            return CorrectAudioTimeStampByVideo(packet);
        }
    }
    return 0;
}

uint32_t TimeCorrector::CorrectAudioTimeStampByVideo(const PacketPtr &packet)
{
    // 首先增加音频包计数
    ++audio_numbers_between_video_;

    // 如果有音频包，就以音频包为基准解析，否则使用视频的基准解析
    if(audio_numbers_between_video_ > 1)
    {
        return CorrectAudioTimeStampByAudio(packet);
    }

    int64_t time = packet->TimeStamp();

    if(video_original_timestamp_ == -1)
    {
        audio_original_timestamp_ = time;
        audio_corrected_timestamp_ = time;
        return time;
    }

    int64_t delta = time - video_original_timestamp_;
    bool fine = (delta>-kMaxVideoDeltaTime)&&(delta<kMaxVideoDeltaTime);
    if(!fine)
    {
        delta = kDefaultVideoDeltaTime;
    }

    audio_original_timestamp_ = time;
    audio_corrected_timestamp_ = video_corrected_timestamp_ + delta;
    if(audio_corrected_timestamp_ < 0)
    {
        audio_corrected_timestamp_ = 0;
    }
    return audio_corrected_timestamp_;
}
uint32_t TimeCorrector::CorrectVideoTimeStampByVideo(const PacketPtr &packet)
{
    audio_numbers_between_video_ = 0;
    int64_t time = packet->TimeStamp();
    if(video_original_timestamp_ == -1)
    {
        video_original_timestamp_ = time;
        video_corrected_timestamp_ = time;

        if(audio_original_timestamp_ != -1)
        {
            int32_t delta = audio_original_timestamp_ - video_original_timestamp_;
            if(delta<=-kMaxVideoDeltaTime||delta >=kMaxVideoDeltaTime)
            {
                video_original_timestamp_ = audio_original_timestamp_;
                video_corrected_timestamp_ = audio_corrected_timestamp_;
            }
        }
    }

    int64_t delta = time - video_original_timestamp_;
    bool fine = (delta>-kMaxVideoDeltaTime)&&(delta<kMaxVideoDeltaTime);
    if(!fine)
    {
        delta = kDefaultVideoDeltaTime;
    }

    video_original_timestamp_ = time;
    video_corrected_timestamp_ += delta;
    if(video_corrected_timestamp_ < 0)
    {
        video_corrected_timestamp_ = 0;
    }
    return video_corrected_timestamp_;
}
uint32_t TimeCorrector::CorrectAudioTimeStampByAudio(const PacketPtr &packet)
{
    int64_t time = packet->TimeStamp();

    if(audio_original_timestamp_ == -1)
    {
        audio_original_timestamp_ = time;
        audio_corrected_timestamp_ = time;
        return time;
    }

    int64_t delta = time - audio_original_timestamp_;
    bool fine = (delta>-kMaxAudioDeltaTime)&&(delta<kMaxAudioDeltaTime);
    if(!fine)
    {
        delta = kDefaultAudioDeltaTime;
    }

    audio_original_timestamp_ = time;
    audio_corrected_timestamp_ += delta;
    if(audio_corrected_timestamp_ < 0)
    {
        audio_corrected_timestamp_ = 0;
    }
    return audio_corrected_timestamp_;
}