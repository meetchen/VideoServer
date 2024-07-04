/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 11:19:14
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:51:18
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFAny.cpp
 * @Description: 所有amf的基类 提供通用的方法
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFAny.h"


using namespace vdse::mmedia;

namespace
{
    static std::string string_null;
}


AMFAny::AMFAny(std::string &name)
:name_(name)
{

}

AMFAny::AMFAny()
{

}

AMFAny::~AMFAny()
{

}

bool AMFAny::Boolean()
{
    if (this -> IsBoolean())
    {
        return this -> Boolean();
    }
    else
    {
        RTMP_DEBUG << " not a boolean ";
        return false;
    }
}

std::string AMFAny::String()
{
    if (this -> IsString())
    {
        return this -> String();
    }
    RTMP_DEBUG << " not a string ";

    return string_null;
}
double AMFAny::Number()
{
    if (this -> IsNumber())
    {
        return this -> Number();
    }
    else
    {
        RTMP_DEBUG << " not a number ";
        return -1.0f;
    }
}

double AMFAny::Date()
{
    if (this -> IsDate())
    {
        return this -> Date();
    }
    else
    {
        RTMP_ERROR << " not a date";
        return -1.0f;
    }
}

AMFObjectPtr AMFAny::Object()
{
    if (this -> IsObject())
    {
        return this -> Object();
    }

    RTMP_ERROR << " not a Object";
    return AMFObjectPtr();
}

bool AMFAny::IsString()
{
    return false;
}
bool AMFAny::IsNumber()
{
    return false;

}
bool AMFAny::IsBoolean()
{
    return false;

}
bool AMFAny::IsDate()
{
    return false;

}
bool AMFAny::IsObject()
{
    return false;

}
bool AMFAny::IsNull()
{
    return false;

}

const std::string &AMFAny::Name() const
{
    return name_;
}

int32_t AMFAny::Count() const
{
    return 1;
}

std::string AMFAny:: DecodeString(const char *data)
{

    auto len = BytesReader::ReadUint16T(data);
    
    if (len > 0)
    {
        std::string str(data + 2, len);
        return str;
    }
    return string_null;
}


// int32_t AMFAny::EncodeNumber(char *output, double dVal)
// {

// }
// int32_t AMFAny::EncodeString(char *output, const std::string& str)
// {

// }
//  int32_t AMFAny::EncodeBoolean(char *output, bool b)
// {

// }
//  int32_t AMFAny::EncodeNamedNumber(char *output, const std::string &name, double dVal)
// {

// }
//  int32_t AMFAny::EncodeNamedString(char *output, const std::string &name, const std::string &value)
// {
    
// }
//  int32_t AMFAny::EncodeNamedBoolean(char *output, const std::string &name, bool bVal)
// {

// }