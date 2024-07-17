/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 16:35:16
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 11:12:35
 * @FilePath: /VideoServer/src/live/PlayerUser.h
 * @Description: 消费视频流(提供给子类一个纯虚函数) 时间戳校正
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "live/User.h"
#include "mmedia/base/Packet.h"
#include "live/base/TimeCorrector.h"
#include <memory>

namespace vdse
{
    namespace live
    {
        using namespace vdse::mmedia;

        class PlayerUser:public User
        {
        public:
            friend class Stream;
            // 委托构造函数
            // using User::User;
            explicit PlayerUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s);

            PacketPtr Meta() const;
            PacketPtr VideoHeader() const;
            PacketPtr AudioHeader() const;
            void ClearMeta();
            void ClearAudioHeader();
            void ClearVideoHeader();  

            virtual bool PostFrames() = 0;
            TimeCorrector& GetTimeCorrector();
        protected:
            PacketPtr video_header_;   
            PacketPtr audio_header_;  
            PacketPtr meta_;  

            bool wait_meta_{true}; 
            bool wait_audio_{true};  
            bool wait_video_{true}; 

            int32_t video_header_index_{0};
            int32_t audio_header_index_{0};
            int32_t meta_index_{0};

            TimeCorrector time_corrector_;
            bool wait_timeout_{false};
            int32_t out_version_{-1}; // 正在输出的流版本号
            int32_t out_frame_timestamp_{0}; //当前发送帧的时间戳
            std::vector<PacketPtr> out_frames_;
            int32_t out_index_{-1};
        };
    }
}