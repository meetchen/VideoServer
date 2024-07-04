/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 14:42:41
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 20:06:45
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFObject.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/rtmp/amf/AMFObject.h"
#include "mmedia/rtmp/amf/AMFBoolean.h"
#include "mmedia/rtmp/amf/AMFDate.h"
#include "mmedia/rtmp/amf/AMFLongString.h"
#include "mmedia/rtmp/amf/AMFString.h"
#include "mmedia/rtmp/amf/AMFNumber.h"




using namespace vdse::mmedia;

namespace
{
    static AMFAnyPtr any_ptr_nullptr;
}

AMFObject::AMFObject(std::string &name)
:AMFAny(name)
{

}

AMFObject::AMFObject()
{

}

AMFObject::~AMFObject()
{

}

int AMFObject::Decode(const char *data, int size, bool has) 
{ 
    // RTMP_TRACE << " AMFObject Decode start ...  size : " << size;
    std::string nname;
    int32_t parsed = 0;

    while (parsed + 3 < size)
    {
        if (BytesReader::ReadUint24T(data) == 0x000009)
        {
            parsed += 3;
            return parsed;
        }
        if (has)
        {
            nname = DecodeString(data);
            parsed += (nname.size() + 2);
            data += (nname.size() + 2);
        }

        int type = *data++;
        // RTMP_TRACE << " decode type : " << type << " parsed : " << parsed;

        parsed++;

        switch(type) 
        {
            case kAMFBoolean:
            {
                auto p = std::make_shared<AMFBoolean>(nname);
                int ret = p ->Decode(data, size - parsed, has);
                if (ret == -1)
                {
                    return -1;
                }
                data += ret;
                parsed += ret;
                RTMP_TRACE << " AMF Boolean " << p ->Boolean();
                properties_.push_back(std::move(p));

                break;
            }
            case kAMFDate:
            {
                auto p = std::make_shared<AMFDate>(nname);
                int ret = p->Decode(data, size - parsed, has);
                if (ret == -1)
                {
                    return -1;
                }
                data += ret;
                parsed += ret;
                RTMP_TRACE << " AMF Date " << p ->Date();
                properties_.push_back(std::move(p));

                break;
            }
            case kAMFString:
            {
                auto p = std::make_shared<AMFString>(nname);
                int ret = p->Decode(data, size - parsed, has);
                if (ret == -1)
                {
                    return -1;
                }
                data += ret;
                parsed += ret;
                RTMP_TRACE << " AMF String " << p ->String();

                properties_.push_back(std::move(p));

                break;
            }
            case kAMFLongString:
            {
                auto p = std::make_shared<AMFString>(nname);
                int ret = p->Decode(data, size - parsed, has);
                if (ret == -1)
                {
                    return -1;
                }
                data += ret;
                parsed += ret;
                RTMP_TRACE << " AMF Long String " << p ->String();
                properties_.push_back(std::move(p));

                break;
            }
            case kAMFObject:
            {
                auto p = std::make_shared<AMFObject>(nname);
                int ret = p->Decode(data, size - parsed, true);
                if (ret == -1)
                {
                    return -1;
                }
                data += ret;
                parsed += ret;
                RTMP_TRACE << " AMF Object ";
                p -> Dump();
                properties_.push_back(std::move(p));
                
                break;
            }
            case kAMFNull:
            {
                RTMP_TRACE << " NUll ";
                break;
            }
            case kAMFEcmaArray:
            {
                auto count = BytesReader::ReadUint32T(data);
                data += 4;
                parsed += 4;
                for (int i = 0; i < count; i++)
                {
                    auto ret = DecodeOnce(data, size - parsed, has);
                    data += ret;
                    parsed += ret;
                }
                RTMP_TRACE << " kAMFEcmaArray ,cout : " << count;

                break;
            }
            case kAMFObjectEnd:
            {
                RTMP_TRACE << " AMF End";
                return parsed;
            }
            case kAMStrictArray:
            {
                auto count = BytesReader::ReadUint32T(data);
                data += 4;
                parsed += 4;
                for (int i = 0; i < count; i++)
                {
                    auto nnumber = std::make_shared<AMFNumber>(nname);
                    int ret = nnumber->Decode(data, size - parsed, has);
                    if (ret == -1)
                    {
                        RTMP_ERROR << "number decode erron";
                        return -1;
                    }
                    data += ret;
                    parsed += ret;
                    properties_.push_back(std::move(nnumber));
                }
                RTMP_TRACE << " kAMFEcmaArray ,cout : " << count;

                break;               
            }
            case kAMFNumber:
            {
                auto p = std::make_shared<AMFNumber>(nname);
                int ret = p->Decode(data, size - parsed, has);
                if (ret == -1)
                {
                    return -1;
                }
                data += ret;
                parsed += ret;
                RTMP_TRACE << " AMF Number " << p ->Number();
                properties_.push_back(std::move(p));

                break;
            }
            default:
            {
                RTMP_TRACE << " unsupport type : " << type << " \n"; 
                break;
            }
        }
    }
    return parsed;
}

AMFObjectPtr AMFObject::Object() 
{
    return std::dynamic_pointer_cast<AMFObject>(shared_from_this());
}

bool AMFObject::IsObject() 
{
    return true;
}

void AMFObject::Dump() const 
{
    RTMP_TRACE << " --------- Object start --------- ";

    for (auto const &p : properties_)
    {
        p->Dump();
    }
    
    RTMP_TRACE << " --------- Object End --------- \n";

}


int AMFObject::DecodeOnce(const char *data, int size, bool has)
{
    std::string nname;
    int32_t parsed = 0;


    if (has)
    {
        nname = DecodeString(data);
        parsed += nname.size() + 2;
        data += nname.size() + 2;
    }

    uint8_t type = *data++;
    parsed++;
    switch(type) 
    {
        case kAMFBoolean:
        {
            auto p = std::make_shared<AMFBoolean>(nname);
            int ret = p ->Decode(data, size - parsed, has);
            if (ret == -1)
            {
                return -1;
            }
            data += ret;
            parsed += ret;
            RTMP_TRACE << " AMF Boolean " << p ->Boolean();
            properties_.push_back(std::move(p));
            break;
        }
        case kAMFDate:
        {
            auto p = std::make_shared<AMFDate>(nname);
            int ret = p->Decode(data, size - parsed, has);
            if (ret == -1)
            {
                return -1;
            }
            data += ret;
            parsed += ret;
            RTMP_TRACE << " AMF Date " << p ->Date();
            properties_.push_back(std::move(p));
            break;
        }
        case kAMFString:
        {
            auto p = std::make_shared<AMFString>(nname);
            int ret = p->Decode(data, size - parsed, has);
            if (ret == -1)
            {
                return -1;
            }
            data += ret;
            parsed += ret;
            RTMP_TRACE << " AMF String " << p ->String();
            properties_.push_back(std::move(p));
            break;
        }
        case kAMFLongString:
        {
            auto p = std::make_shared<AMFString>(nname);
            int ret = p->Decode(data, size - parsed, has);
            if (ret == -1)
            {
                return -1;
            }
            data += ret;
            parsed += ret;
            RTMP_TRACE << " AMF Long String " << p ->String();
            properties_.push_back(std::move(p));
            break;
        }
        case kAMFObject:
        {
            auto p = std::make_shared<AMFObject>(nname);
            int ret = p->Decode(data, size - parsed, true);
            if (ret == -1)
            {
                return -1;
            }
            data += ret;
            parsed += ret;
            RTMP_TRACE << " AMF Object ";
            properties_.push_back(std::move(p));
            p -> Dump();
            break;
        }
        case kAMFNull:
        {
            RTMP_TRACE << " NUll ";
            break;
        }
        case kAMFEcmaArray:
        {
            auto count = BytesReader::ReadUint32T(data);
            data += 4;
            parsed += 4;
            for (int i = 0; i < count; i++)
            {
                auto ret = DecodeOnce(data, size - parsed, has);
                data += ret;
                parsed += ret;
            }
            RTMP_TRACE << " kAMFEcmaArray ,cout : " << count;

            break;
        }
        case kAMFObjectEnd:
        {
            RTMP_TRACE << " AMF End";
            return parsed;
        }
        case kAMStrictArray:
        {
            auto count = BytesReader::ReadUint32T(data);
            data += 4;
            parsed += 4;
            for (int i = 0; i < count; i++)
            {
                auto nnumber = std::make_shared<AMFNumber>(nname);
                int ret = nnumber->Decode(data, size - parsed, has);
                if (ret == -1)
                {
                    RTMP_ERROR << "number decode erron";
                    return -1;
                }
                data += ret;
                parsed += ret;
                properties_.push_back(std::move(nnumber));
            }
            RTMP_TRACE << " kAMFEcmaArray ,cout : " << count;

            break;               
        }
        case kAMFNumber:
        {
            auto p = std::make_shared<AMFNumber>(nname);
            int ret = p->Decode(data, size - parsed, has);
            if (ret == -1)
            {
                return -1;
            }
            data += ret;
            parsed += ret;
            RTMP_TRACE << " AMF Number " << p ->Number();
            properties_.push_back(std::move(p));
            break;
        }
        default:
        {
            RTMP_TRACE << "unsupport type : " << type;
            break;
        }
    }
    return parsed;
}

const AMFAnyPtr &AMFObject::Property(const std::string &name) const
{
    for (const auto&p : properties_)
    {
        if (p -> Name() == name)
        {
            return p;
        }
        else if (p -> IsObject())
        {
            auto obj = p -> Object();
            const auto& res = obj -> Property(name);
            if (res)
            {
                return res;
            }
        }
    }
    return any_ptr_nullptr;
}

const AMFAnyPtr &AMFObject::Property(int index) const
{
    if (index > properties_.size() || index < 0)
    {
        RTMP_ERROR << " out of size, index : " << index << " size : " << properties_.size();
        return any_ptr_nullptr;
    }
    return properties_[index];
}