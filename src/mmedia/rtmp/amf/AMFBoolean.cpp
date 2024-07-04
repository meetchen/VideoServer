/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:42:41
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:03:51
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFBoolean.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFBoolean.h"

using namespace vdse::mmedia;


AMFBoolean::AMFBoolean(std::string &name)
:AMFAny(name)
{

}

AMFBoolean::AMFBoolean()
{

}

AMFBoolean::~AMFBoolean()
{

}

int AMFBoolean::Decode(const char *data, int size, bool has) 
{
    if (size >= 1)
    {
        boolean_ = *data != 0 ? true : false;
        return 1;
    }
    return -1;

}

bool AMFBoolean::Boolean() 
{
    return boolean_;
}

bool AMFBoolean::IsBoolean() 
{
    return true;
}

void AMFBoolean::Dump() const 
{
    RTMP_TRACE << " Boolean : " << boolean_ ;
}
