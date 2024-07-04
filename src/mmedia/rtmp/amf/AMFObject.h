/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 15:09:28
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:02:29
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFObject.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/rtmp/amf/AMFAny.h"
#include <vector>


namespace vdse 
{
    namespace mmedia
    {
        
        class AMFObject : public AMFAny
        {
            public:
                AMFObject(std::string &name);
                AMFObject(); 
                ~AMFObject();

                int Decode(const char *data, int size, bool has = false) override;
                AMFObjectPtr Object() override;
                bool IsObject() override;
                void Dump() const override;

                int DecodeOnce(const char *data, int size, bool has = false);

                const AMFAnyPtr &Property(const std::string &name) const;

                const AMFAnyPtr &Property(int index) const;

            private:
                
                std::string string_;
                std::vector<AMFAnyPtr> properties_;
        };
    }
}