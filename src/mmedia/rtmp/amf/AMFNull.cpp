/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-16 18:28:42
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 18:32:07
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFNull.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by duanran, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFNull.h"

using namespace vdse::mmedia;


AMFNull::AMFNull(std::string &name)
:AMFAny(name)
{

}

AMFNull::AMFNull()
{

}

AMFNull::~AMFNull()
{

}

int AMFNull::Decode(const char *data, int size, bool has) 
{
    return 0;
}


bool AMFNull::IsNull() 
{
    return true;
}

void AMFNull::Dump() const 
{
    RTMP_TRACE << " Null ";
}