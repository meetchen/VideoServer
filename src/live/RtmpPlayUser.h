/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-15 14:11:13
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 11:13:29
 * @FilePath: /VideoServer/src/live/RtmpPlayUser.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${995122760@qq.com}, All Rights Reserved. 
 */
#pragma once
#include "mmedia/base/Packet.h"
#include "live/PlayerUser.h"


namespace vdse
{
    namespace live
    {
        class RtmpPlayUser : public PlayerUser
        {
            public:
                
                RtmpPlayUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s);
                UserType GetUserType() const override;
                bool PostFrames();

            private:
                using User::SetUserType;
                bool PushFrame(PacketPtr & packet, bool is_header);
                bool PushFrames(std::vector<PacketPtr> & packets);
        };
    }
}