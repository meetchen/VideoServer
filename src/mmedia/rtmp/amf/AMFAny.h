/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-04 11:17:10
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-04 19:03:02
 * @FilePath: /VideoServer/src/mmedia/rtmp/amf/AMFAny.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include "mmedia/base/BytesReader.h" 
#include "mmedia/base/MMediaLog.h"

namespace vdse
{
    namespace mmedia
    {

        enum AMFDataType
        { 
            kAMFNumber = 0, 
            kAMFBoolean, 
            kAMFString, 
            kAMFObject,
            kAMFMovieClip,		/* reserved, not used */
            kAMFNull, 
            kAMFUndefined, 
            kAMFReference, 
            kAMFEcmaArray, 
            // MixedArray
            kAMFObjectEnd,
            kAMStrictArray, 
            kAMFDate, 
            kAMFLongString, 
            kAMFUnsupported,
            kAMFRecordset,		/* reserved, not used */
            kAMFXMLDoc, 
            kAMFTypedObject,
            kAMFAvmplus,		/* switch to AMF3 */
            kAMFInvalid = 0xff,
        }; 

        class AMFObject;
        using AMFObjectPtr = std::shared_ptr<AMFObject>;
        class AMFAny;
        using AMFAnyPtr = std::shared_ptr<AMFAny>;

        class AMFAny : public std::enable_shared_from_this<AMFAny>
        {
            public:
                AMFAny(std::string &name);
                AMFAny();

                virtual ~AMFAny();

                /**
                 * @description: 解析数据
                 * @param {char} *data 需要解析的数据地址
                 * @param {int} size 需要解析的数据大小
                 * @param {bool} has 是否带名字
                 * @return {int} >= 解析成功的字节数 ， -1 解析失败
                 */
                virtual int Decode(const char *data, int size, bool has = false) = 0;

                virtual bool Boolean();
                virtual double Number();
                virtual double Date();
                virtual AMFObjectPtr Object();
                virtual std::string String();

                virtual bool IsString();
                virtual bool IsNumber();
                virtual bool IsBoolean();
                virtual bool IsDate();
                virtual bool IsObject();
                virtual bool IsNull();

                /**
                 * @description: 子类弹出一个数据，对应不同的类型
                 * @return {*}
                 */                
                virtual void Dump() const = 0;

                const std::string &Name() const;

                /**
                 * @description: 返回当前类型有几个属性
                 * @return {*}
                 */                
                virtual int32_t Count() const;

                std::string DecodeString(const char *data);


                static int32_t EncodeNumber(char *output, double dVal);
                static int32_t EncodeString(char *output, const std::string& str);
                static int32_t EncodeBoolean(char *output, bool b);
                static int32_t EncodeNamedNumber(char *output, const std::string &name, double dVal);
                static int32_t EncodeNamedString(char *output, const std::string &name, const std::string &value);
                static int32_t EncodeNamedBoolean(char *output, const std::string &name, bool bVal);
                

            private:

                std::string name_;
        };

    }
}