/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:42:41
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:04:02
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFDate.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFDate.h"

using namespace vdse::mmedia;


AMFDate::AMFDate(std::string &name)
:AMFAny(name)
{

}

AMFDate::AMFDate()
{

}

AMFDate::~AMFDate()
{

}

int AMFDate::Decode(const char *data, int size, bool has) 
{ 
    if (size < 10)
    {
        return -1;
    }
    utc_ = BytesReader::ReadUint64T(data);
    utc_offset_ = BytesReader::ReadUint16T(data + 8);

    return 10;

}
double AMFDate::Date() 
{
    return utc_;
}

int16_t AMFDate::UtcOffset() const
{
    return utc_offset_;
}


bool AMFDate::IsDate() 
{
    return true;
}

void AMFDate::Dump() const 
{
    RTMP_TRACE << " Date : " << utc_ ;
}
