/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-09 15:07:12
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 10:32:31
 * @FilePath: /VideoServer/src/live/base/TimeCorrector.h
 * @Description: 通过原始时间戳修改时间戳，保证时间戳数值稳定递增， 记录本次的时间戳，和上次修改后的时间戳，如果本次的时间戳和之前的时间差
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "mmedia/base/Packet.h"
#include <cstdint>

namespace vdse
{
    namespace live
    {
        using namespace vdse::mmedia;

        class TimeCorrector
        {
            const int32_t kMaxVideoDeltaTime = 100;
            const int32_t kMaxAudioDeltaTime = 100;
            const int32_t kDefaultVideoDeltaTime = 40;
            const int32_t kDefaultAudioDeltaTime = 20; 
            
        public:
            TimeCorrector() = default;
            ~TimeCorrector() = default;
            /**
             * @description: 根据不同的包类型，选择不同的基准进行修改
             * @param {PacketPtr} &packet
             * @return {*}
             */            
            uint32_t CorrectTimestamp(const PacketPtr &packet);
            /**
             * @description: 修改视频时间戳 基于视频的基准
             * @param {PacketPtr} &packet
             * @return {*}
             */            
            uint32_t CorrectAudioTimeStampByVideo(const PacketPtr &packet);
            uint32_t CorrectVideoTimeStampByVideo(const PacketPtr &packet);
            uint32_t CorrectAudioTimeStampByAudio(const PacketPtr &packet);
        private:
            int64_t video_original_timestamp_{-1}; // 原始视频时间戳
            int64_t video_corrected_timestamp_{0};  // 更改后的视频时间戳
            int64_t audio_original_timestamp_{-1};
            int64_t audio_corrected_timestamp_{0};
            int32_t audio_numbers_between_video_{0}; //  两个视频包之间有几个音频包
        };
    }
}