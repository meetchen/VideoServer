/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 22:11:21
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-03 14:25:19
 * @FilePath: /VideoServer/src/mmedia/base/BytesReader.cpp
 * @Description: 从网络字节顺序转换为主机字节顺序 大端 （逆序） -》 小端 （低地址低低内容 正序）
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/base/BytesReader.h"
#include <cstdint>
#include <netinet/in.h>
#include <cstring>

using namespace vdse::mmedia;

uint64_t BytesReader::ReadUint64T(const char *data)
{
    uint64_t in  = *((uint64_t*)data);
    uint64_t res = __bswap_64(in);
    double value;
    memcpy(&value, &res, sizeof(double));
    return value;
}

uint32_t BytesReader::ReadUint32T(const char *data)
{
    uint32_t *c = (uint32_t *) data;
    // ntohl()将一个无符号长整形数从网络字节顺序转换为主机字节顺序
    return ntohl(*c);
}

uint32_t BytesReader::ReadUint24T(const char *data)
{
    unsigned char *c = (unsigned char *) data;
    uint32_t val;
    val = (c[0] << 16) | (c[1] << 8) | c[2];
    return val;
}

uint16_t BytesReader::ReadUint16T(const char *data)
{
    uint16_t *c = (uint16_t *) data;
    return ntohs(*c);
}

uint8_t BytesReader::ReadUint8T(const char *data)
{
    return data[0];
}
