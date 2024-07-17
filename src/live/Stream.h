/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 18:04:16
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 17:53:43
 * @FilePath: /VideoServer/src/live/Stream.h
 * @Description: 完成视频流的录入与推出，缓存
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "live/User.h"
#include "live/PlayerUser.h"
#include "live/GopMgr.h"
#include "live/CodecHeader.h"
#include "live/base/TimeCorrector.h"
#include <thread>
#include <mutex>
#include <string>
#include <atomic>



namespace vdse
{
    namespace live
    {
        using namespace vdse::mmedia;
        using UserPtr = std::shared_ptr<User>;
        using PlayerUserPtr = std::shared_ptr<PlayerUser>;
        class Session;
        class Stream
        {
            public:
                Stream(Session &s,const std::string &session_name);

                int64_t ReadyTime() const ;
                int64_t SinceStart() const ;
                bool Timeout();
                
                /**
                * @description: 获取到第一帧数据的时间
                * @return {*}
                */            
                int64_t DataTime() const ;
                
                const std::string &SessionName() const ;

                int32_t StreamVersion() const;

                /**
                * @description: 有无收到媒体信息
                * @return {*}
                */            
                bool HasMedia() const;

                bool Ready() const;
                
                /**
                * @description: 数据输入
                * @param {PacketPtr &&} packet
                * @return {*}
                */            
                void AddPacket(PacketPtr && packet);

                void GetFrames(const PlayerUserPtr &user);

            private:
                /**
                 * @description: 定位GOP
                 * @param {PlayerUserPtr} &user
                 * @return {bool} 返回是否定位成功
                 */                
                bool LocateGop(const PlayerUserPtr &user);
                /**
                 * @description: 跳帧，追赶实时音视频
                 * @param {PlayerUserPtr} &user
                 * @return {*}
                 */                
                void SkipFrame(const PlayerUserPtr &user);
                /**
                 * @description: 输出一定数量的视频帧
                 * @param {PlayerUserPtr} &user
                 * @return {*}
                 */                
                void GetNextFrame(const PlayerUserPtr &user); 

                void SetReady(bool ready);
                
                int64_t data_coming_time_{0}; // 第一次接收到这个数据流的 时间戳
                
                int64_t start_timestamp_{0}; // 当前Stream对象创建的 时间戳

                int64_t ready_time_{0}; // 收到关键帧的 时间戳

                std::atomic<int64_t> stream_time_{0}; // 每次流数据来的时间 （判断超时）

                Session &session_;

                std::string session_name_; // 会话名

                std::atomic<int64_t> frame_index_{-1};  // 帧的索引

                uint32_t packet_buffer_size_{1000}; // 保存实时音视频数据的缓冲区大小

                std::vector<PacketPtr> packet_buffer_; // 保存实时音视频数据

                bool has_audio_{false}; // 有无音频
                bool has_video_{false};
                bool has_meta_{false};
                bool ready_{false};
                
                std::atomic<int32_t> stream_version_{-1}; // 流版本号

                GopMgr gop_mgr_;
                CodecHeader codec_headers_;
                TimeCorrector time_corrector_;
                std::mutex lock_;
        };
    }
}