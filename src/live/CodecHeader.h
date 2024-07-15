/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 14:25:36
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 14:41:51
 * @FilePath: /VideoServer/src/live/CodecHeader.h
 * @Description: 从推流数据中识别codeheader 从而保证对于解码器等的初始化
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/base/Packet.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace vdse
{
    namespace live
    {
        using namespace vdse::mmedia;

        class CodecHeader
        {
        public:
            CodecHeader();
            ~CodecHeader();

            /**
             * @description: 找到距离idx前面最近的一个header
             * @param {int} idx
             * @return {*}
             */            
            PacketPtr  Meta(int idx);
            PacketPtr  VideoHeader(int idx);
            PacketPtr  AudioHeader(int idx);
            void SaveMeta(const PacketPtr &packet);
            void ParseMeta(const PacketPtr &packet);
            void SaveAudioHeader(const PacketPtr &packet);
            void SaveVideoHeader(const PacketPtr &packet);
            bool ParseCodecHeader(const PacketPtr &packet);

        private:
            PacketPtr video_header_;
            PacketPtr audio_header_;
            PacketPtr meta_;
            int meta_version_{0};
            int audio_version_{0};
            int video_version_{0};
            std::vector<PacketPtr> video_header_packets_;
            std::vector<PacketPtr> audio_header_packets_;
            std::vector<PacketPtr> meta_packets_;
            int64_t start_timestamp_{0};
        };
    }
}