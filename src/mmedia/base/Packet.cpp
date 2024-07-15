/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 16:51:14
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-05 16:08:50
 * @FilePath: /VideoServer/src/mmedia/base/Packet.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/base/Packet.h"
#include <cstring>

using namespace vdse::mmedia;

PacketPtr Packet::NewPacket(int32_t size)
{
    auto block_size = size + sizeof(Packet);
    Packet *packet = (Packet *)new char[block_size];
    memset((void *)packet, 0x00, block_size);
    packet -> type_ = kPacketTypeUnkonwed;
    packet -> index_ = -1;
    packet -> capacity_ = size;
    packet -> ext_.reset();
    return PacketPtr(packet, [](Packet *p){
        delete[] (char *)p;
    });
}