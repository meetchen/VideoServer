/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:29:15
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 18:36:38
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFNull.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/rtmp/amf/AMFAny.h"
#include <string>


namespace vdse
{
    namespace mmedia
    {
        class AMFNull : public AMFAny
        {
            public:
                AMFNull(std::string &name);
                AMFNull(); 
                ~AMFNull();

                int Decode(const char *data, int size, bool has = false) override;
                bool IsNull() override;
                void Dump() const override;

            private:
                
        };
    }
}