/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-13 20:24:31
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-13 20:24:48
 * @FilePath: /VideoServer/src/mmedia/base/NalBitStream.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <cstdint>
namespace vdse
{
    namespace mmedia
    {
        class NalBitStream 
        {
            public:
                NalBitStream(const char *data, int len);
                uint8_t GetBit();
                uint16_t GetWord(int bits);
                uint32_t GetBitLong(int bits);
                uint64_t GetBit64(int bits);
                uint32_t GetUE();
                int32_t GetSE();
            private:
                char GetByte();
                const char * data_;
                int len_;
                int bits_count_;
                int byte_idx_;
                char byte_;
        };
    }
}