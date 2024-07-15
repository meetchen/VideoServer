/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:29:15
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 15:43:38
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFNumber.h
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
        class AMFNumber : public AMFAny
        {
            public:
                AMFNumber(std::string &name);
                AMFNumber(); 
                ~AMFNumber();

                int Decode(const char *data, int size, bool has = false) override;
                double Number() override;
                bool IsNumber() override;
                void Dump() const override;



            private:
                
                double number_{0.0f};
        };
    }
}