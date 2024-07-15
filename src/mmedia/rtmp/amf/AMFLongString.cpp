/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:42:41
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:04:33
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFLongString.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFLongString.h"

using namespace vdse::mmedia;


AMFLongString::AMFLongString(std::string &name)
:AMFAny(name)
{

}

AMFLongString::AMFLongString()
{

}

AMFLongString::~AMFLongString()
{

}

int AMFLongString::Decode(const char *data, int size, bool has) 
{ 
    if (size < 2)
    {
        return -1;
    }
    auto len = BytesReader::ReadUint32T(data);
    if (size - 2 < len || len < 0)
    {
        return -1;
    }
    string_.assign(data + 4, len);
    return len + 4;

}

std::string AMFLongString::String() 
{
    return string_;
}

bool AMFLongString::IsString() 
{
    return true;
}

void AMFLongString::Dump() const 
{
    RTMP_TRACE << "Long String : " << string_ ;
}
