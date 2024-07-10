/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 16:29:13
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 16:35:06
 * @FilePath: /VideoServer/src/live/PublishUser.h
 * @Description: 推流的管理类 注入实时音视频数据
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "live/User.h"
#include <memory>

namespace vdse
{
    namespace live
    {
        class Stream;

        using StreamPtr = std::shared_ptr<Stream>;

        class PublishUser : public User
        {
            public:
                explicit PublishUser(const ConnectionPtr &conn, const StreamPtr &stream)
                :User(conn, stream , nullptr), stream_(stream)
                {

                }
                ~PublishUser() = default;

                StreamPtr Stream()
                {
                    return stream_;
                }
            private:
                StreamPtr stream_;
        };
    }
}