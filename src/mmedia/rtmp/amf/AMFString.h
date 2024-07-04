/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:41:21
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 15:16:12
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFString.h
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
        class AMFString : public AMFAny
        {
            public:
                AMFString(std::string &name);
                AMFString(); 
                ~AMFString();

                int Decode(const char *data, int size, bool has = false) override;
                std::string String() override;
                bool IsString() override;
                void Dump() const override;



            private:
                
                std::string string_{};
        };
    }
}