/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:35:43
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:05:18
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFNumber.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFNumber.h"

using namespace vdse::mmedia;


AMFNumber::AMFNumber(std::string &name)
:AMFAny(name)
{

}

AMFNumber::AMFNumber()
{

}

AMFNumber::~AMFNumber()
{

}

int AMFNumber::Decode(const char *data, int size, bool has) 
{
    if (size >= 8)
    {
        number_ = BytesReader::ReadUint64T(data);
        return 8;
    }
    return -1;

}

double AMFNumber::Number() 
{
    return number_;
}

bool AMFNumber::IsNumber() 
{
    return true;
}

void AMFNumber::Dump() const 
{
    RTMP_TRACE << " Number : " << number_ ;
}