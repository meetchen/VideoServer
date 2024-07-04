/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 16:45:59
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-03 16:22:26
 * @FilePath: /VideoServer/src/mmedia/base/Packet.h
 * @Description: 用于表示各类数据包、音频包、视频包、Meta包、等其他数据包
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/base/Packet.h"
#include <memory>


namespace vdse
{
    namespace mmedia
    {

        enum
        {
            kPacktorTypeVideo = 1,
            kPacktorTypeAudio = 2,
            kPacktorTypeMeta = 4,
            kPacktorTypeMeta3 = 8,
            kFrameTypeKeyFrame = 16,
            kFrameTypeIDR = 32,
            kPacktorTypeUnkonwed = 255,
        };

        class Packet;
        using PacketPtr = std::shared_ptr<Packet>;
#pragma pack(push)
#pragma pack(1)

        class Packet
        { 
            public:
                Packet(int32_t size):capacity_(size)
                {

                }

                ~Packet() {}
                /**
                 * @brief        :  创建一个新的packet，
                 * @param         {int32_t} size:指定数据区大小
                 * @return        {*}
                **/                
                static PacketPtr NewPacker(int32_t size);

                bool IsVideo() const
                {
                    return (type_ & kPacktorTypeVideo) == kPacktorTypeVideo;
                }
                bool IsAudio() const
                {
                    return type_ == kPacktorTypeAudio;
                }
                bool IsKeyFrame() const
                {
                    return IsVideo() && ((type_ & kFrameTypeKeyFrame) == kFrameTypeKeyFrame);
                }
                bool IsMeta() const
                {
                    return type_ == kPacktorTypeMeta;
                }
                bool IsMeta3() const
                {
                    return type_ == kPacktorTypeMeta3;
                }
                
                inline int32_t PacketSize() const
                {
                    return size_;
                }

                inline int Space() const
                {
                    return capacity_ - size_;
                }

                inline void SetPacketSize(size_t size)
                {
                    size_ = size;
                }

                inline void UpDatePackSize(size_t size)
                {
                    size_ += size;
                }

                void SetIndex(int32_t index)
                {
                    index_ = index;
                }

                int32_t Index() const
                {
                    return index_;
                }

                void SetPacketType(int32_t type) 
                {
                    type_ = type;
                }
                
                int32_t PacketType() const
                {
                    return type_;
                }

                void SetTimeStamp(uint64_t timestamp)
                {
                    timestamp_ = timestamp;
                }

                uint64_t TimeStamp() const
                {
                    return timestamp_;
                }

                inline char* Data()
                {
                    return (char *)this + sizeof(Packet);
                }


                template <typename T>
                inline std::shared_ptr<T> Ext() const
                {
                    return std::static_pointer_cast<T>(ext_);
                }

                inline void SetExt(const std::shared_ptr<void> &ext)
                {
                    ext_ = ext;
                }
                
            private:
                uint32_t capacity_{0};
                uint32_t size_{0};
                int32_t index_{-1};
                int32_t type_{kPacktorTypeUnkonwed};
                std::shared_ptr<void> ext_;
                uint64_t timestamp_{0};


        };
#pragma pack()

    }
}

