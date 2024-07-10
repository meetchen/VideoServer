/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 15:25:42
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 18:58:24
 * @FilePath: /VideoServer/src/live/GopMgr.h
 * @Description:  关于GOP，即两个I帧之间的所有帧，因为视频发送的首帧需要是I帧
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */

#pragma once


#include <vector>
#include "mmedia/base/Packet.h"


namespace vdse
{
    namespace live
    {
        using namespace vdse::mmedia;

        struct GopItemInfo
        {
            int32_t index;
            int64_t timestamp;

            GopItemInfo(int32_t i, int64_t t)
            :index(i), timestamp(t)
            {

            }
        };

        class GopMgr
        {
            public:
                GopMgr() = default;
                ~GopMgr() = default;
                /**
                 * @description: 收到一帧，调用packet进行检查管理，更新gop信息
                 * @param {PacketPtr} &packet
                 * @return {*}
                 */                
                void AddFrame(const PacketPtr &packet);
                int32_t MaxGopLength() const;
                size_t GopSize() const;
                /**
                 * @description: 寻找距离预定的延迟最近的 gop
                 * @param {int} content_latency 预定的延迟
                 * @param {int} &latency 查找到最优延迟
                 * @return {int} gop id
                 */                
                int GetGopByLatency(int content_latency, int &latency) const;
                /**
                 * @description: 清理过期GOP
                 * @param {int} min_idx 小于该index gop被清理
                 * @return {*}
                 */                
                void ClearExpriedGop(int min_idx);
                void PrintAllGop();
                int64_t LastestTimeStamp() const
                {
                    return lastest_timestamp_;
                }
            private:
            
                std::vector<GopItemInfo> gops_;
                int32_t gop_length_{0};
                int32_t max_gop_length_{0};
                int32_t gop_number_{0};
                int32_t total_gop_length_{0};
                int64_t lastest_timestamp_{0};
        };
    }
}

