/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 11:19:14
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-05 17:51:15
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFAny.cpp
 * @Description: 所有amf的基类 提供通用的方法
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFAny.h"
#include "mmedia/base/BytesWriter.h"
#include <cstdio>
#include <cstring>


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
    std::cout  << "DecodeSting len : " << len << std::endl;
    // std::printf(" data: %s", data + 2);
    // std::cout  << "-----" << std::endl;

    if (len > 0)
    {
        return std::string(data + 2, len);
    }
    return string_null;
}


int32_t AMFAny::EncodeNumber(char *output, double dVal)
{
    char *p = output;

    *p++ = kAMFNumber;

    p += BytesWriter::WriteDouble64T(p, dVal);

    return p - output;
}

int32_t AMFAny::EncodeString(char *output, const std::string& str)
{
    char *p = output;

    *p++ = kAMFString;

    uint16_t size = str.size();

    p += BytesWriter::WriteUint16T(p, size);

    memcpy(p, str.c_str(), size);

    p += size;

    return p - output;
}

int32_t AMFAny::EncodeBoolean(char *output, bool b)
{
    char *p = output;

    *p++ = kAMFBoolean;

    *p++ = b ? 0x01 : 0x00;

    return p - output;
}
int32_t AMFAny::EncodeNamedNumber(char *output, const std::string &name, double dVal)
{
    char *p = output;


    p += EncodeName(p, name);

    p += EncodeNumber(p, dVal);


    return p - output;
}
 int32_t AMFAny::EncodeNamedString(char *output, const std::string &name, const std::string &value)
{
    char *p = output;


    p += EncodeName(p, name);

    p += EncodeString(p, value);

    return p - output;
}
 int32_t AMFAny::EncodeNamedBoolean(char *output, const std::string &name, bool bVal)
{
    char *p = output;


    p += EncodeName(p, name);

    p += EncodeBoolean(p, bVal);

    return p - output;
    
}
int32_t AMFAny::EncodeName(char *output, const std::string &name)
{
    char *p = output;

    uint16_t size = name.size();

    p += BytesWriter::WriteUint16T(p, size);

    memcpy(p, name.c_str(), size);

    p += size;

    return p - output;
}