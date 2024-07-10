/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-09 15:31:02
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-09 15:35:27
 * @FilePath: /VideoServer/src/live/base/CodecUtils.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/base/Packet.h"


namespace vdse
{
    namespace live
    {
        using namespace vdse::mmedia;

        class CodecUtils
        {
            public:
                static bool IsCodecHeader(const PacketPtr &packet);
        };
    }
}
