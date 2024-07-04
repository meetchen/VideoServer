/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:41:21
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 15:16:34
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFBoolean.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/rtmp/amf/AMFAny.h"


namespace vdse
{
    namespace mmedia
    {
        class AMFBoolean : public AMFAny
        {
            public:
                AMFBoolean(std::string &name);
                AMFBoolean(); 
                ~AMFBoolean();

                int Decode(const char *data, int size, bool has = false) override;
                bool Boolean() override;
                bool IsBoolean() override;
                void Dump() const override;



            private:
                
                bool boolean_{false};
        };
    }
}