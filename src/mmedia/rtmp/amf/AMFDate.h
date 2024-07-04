/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:41:21
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 15:43:54
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFDate.h
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
        class AMFDate : public AMFAny
        {
            public:
                AMFDate(std::string &name);
                AMFDate(); 
                ~AMFDate();

                int Decode(const char *data, int size, bool has = false) override;
                double Date() override;
                bool IsDate() override;
                void Dump() const override;
                int16_t UtcOffset() const;

            private:
                
                double utc_{0.0f};
                int16_t utc_offset_{0};
        };
    }
}