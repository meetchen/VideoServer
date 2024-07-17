/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-15 10:54:03
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 14:51:35
 * @FilePath: /VideoServer/src/live/base/CodecUtils.cpp
 * @Description: 判断是否是关键帧等
 * 
 * Copyright (c) 2024 by duanran, All Rights Reserved. 
 */
#include "live/base/CodecUtils.h"


using namespace vdse::live;


bool CodecUtils::IsCodecHeader(const PacketPtr &packet)
{
    if (packet->PacketSize() > 1)
    {
        auto p = packet->Data() + 1;
        if (*p == 0)
        {
            return true;
        }
    }
    return false;
}


bool CodecUtils::IsKeyFrame(const PacketPtr &packet)
{
    if (packet->PacketSize() > 0)
    {
        auto p = packet->Data();
        return ((*p>>4)&0x0f) == 1;
    }
    return false;
}