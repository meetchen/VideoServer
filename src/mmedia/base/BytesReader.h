/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-02 22:11:22
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-03 14:18:14
 * @FilePath: /VideoServer/src/mmedia/base/BytesReader.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include <stdint.h>

namespace vdse
{
    namespace mmedia
    {
        class BytesReader
        {
        public:
            BytesReader() = default;
            ~BytesReader() = default;
            static uint64_t ReadUint64T(const char *data);
            static uint32_t ReadUint32T(const char *data);
            static uint32_t ReadUint24T(const char *data);
            static uint16_t ReadUint16T(const char *data);
            static uint8_t  ReadUint8T(const char *data);
        }; 
    }   
}
