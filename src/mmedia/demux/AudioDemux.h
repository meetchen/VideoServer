/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-13 20:25:55
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-13 20:26:17
 * @FilePath: /VideoServer/src/mmedia/demux/demux/AudioDemux.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "mmedia/base/AVTypes.h"
#include <list>
#include <cstdint>
#include <string>

namespace vdse
{
    namespace mmedia
    {
        class AudioDemux
        {
        public:
            AudioDemux() = default;
            ~AudioDemux() = default;

            int32_t OnDemux(const char *data,size_t size,std::list<SampleBuf> &list);
            int32_t GetCodecId()const
            {
                return sound_format_;
            }
            AACObjectType GetObjectType() const 
            {
                return aac_object_;
            }
            int32_t GetSampleRateIndex() const 
            {
                return aac_sample_rate_;
            }
            uint8_t GetChannel() const 
            {
                return aac_channel_;
            }
            int32_t GetSampleRate() const;
            const std::string &AACSeqHeaer() const;
        private:            
            int32_t DemuxAAC(const char *data,size_t size,std::list<SampleBuf> &list);
            int32_t DemuxMP3(const char *data,size_t size,std::list<SampleBuf> &);
            int32_t DemuxAACSequenceHeader(const char* data, int size);

            int32_t sound_format_;
            int32_t sound_rate_;
            int32_t sound_size_;
            int32_t sound_type_;
            AACObjectType aac_object_;
            int32_t aac_sample_rate_;
            uint8_t aac_channel_;
            bool aac_ok_{false};
            std::string aac_seq_header_;
        };
    }
}