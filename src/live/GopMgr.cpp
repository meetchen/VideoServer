/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 15:36:52
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 16:16:41
 * @FilePath: /VideoServer/src/live/GopMgr.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "live/GopMgr.h"
#include <sstream>
#include "live/base/LiveLog.h"


using namespace vdse::live;


void GopMgr::AddFrame(const PacketPtr &packet)
{
    lastest_timestamp_ = packet -> TimeStamp();
    if (packet -> IsKeyFrame())
    {
        LIVE_TRACE << " insert key frame , index = " << packet->Index() << " ts : " << packet -> TimeStamp();
        gops_.emplace_back(packet->Index(), packet->TimeStamp());
        max_gop_length_ = std::max(max_gop_length_ , gop_length_);
        total_gop_length_ += gop_length_;
        gop_length_ = 0;
        gop_number_ ++;
    }
    gop_length_++;
}

int32_t GopMgr::MaxGopLength() const
{
    return max_gop_length_;
}

size_t GopMgr::GopSize() const
{
    return gops_.size(); 
}

int GopMgr::GetGopByLatency(int content_latency, int &latency) const
{
    int pos = -1;

    latency = 0;

    for (auto it = gops_.rbegin(); it != gops_.rend(); ++it)
    {
        auto item_latency = lastest_timestamp_ - it -> timestamp ;
        if (item_latency <= content_latency)
        {
            pos = it -> index;
            latency = item_latency;
        }
        else
        {
            break;
        }
    }
    return pos;
}

void GopMgr::ClearExpriedGop(int min_idx)
{
    if (gops_.empty())
    {
        return;
    }

    for (auto it = gops_.begin(); it != gops_.end();)
    {
        if (it -> index <= min_idx)
        {
            it = gops_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void GopMgr::PrintAllGop()
{
    std::stringstream ss;
    ss << "all gops";
    for (auto it = gops_.begin(); it != gops_.end(); ++it)
    {
        ss << "[" << it -> index << "," << it ->timestamp << "]";
    }
    LIVE_TRACE << ss.str() << "\n";
}
