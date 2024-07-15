/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:42:41
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-05 16:44:15
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFString.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFString.h"

using namespace vdse::mmedia;


AMFString::AMFString(std::string &name)
:AMFAny(name)
{

}

AMFString::AMFString()
{

}

AMFString::~AMFString()
{

}

int AMFString::Decode(const char *data, int size, bool has) 
{ 

    if (size < 2)
    {
        return -1;
    }
    auto len = BytesReader::ReadUint16T(data);


    if (size - 2 < len || len < 0)
    {
        return -1;
    }

    string_ = AMFAny::DecodeString(data) ;

    return len + 2;

}

std::string AMFString::String() 
{
    return string_;
}

bool AMFString::IsString() 
{
    return true;
}

void AMFString::Dump() const 
{
    RTMP_TRACE << " String : " << string_ ;
}
