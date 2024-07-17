/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-15 15:11:23
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 11:25:20
 * @FilePath: /VideoServer/src/live/Session.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${user.name}, All Rights Reserved. 
 */
#pragma once
#include "live/User.h"
#include "live/PlayerUser.h"
#include "base/AppInfo.h"
#include "live/Stream.h"
#include <string>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <memory>

namespace vdse
{
    namespace live
    {
        class Session : public std::enable_shared_from_this<Session>
        {
            
            public:
                explicit Session(const std::string& session_name_);

                int64_t ReadyTime() const;
                ino64_t SinceStart() const;
                bool IsTimeOut();
                UserPtr CreatePublisher(const std::string &session_name, const ConnectionPtr &conn, UserType type,  const std::string &param);
                UserPtr CreatePlayer(const std::string &session_name, const ConnectionPtr &conn, UserType type,  const std::string &param);
                void CloseUser(const UserPtr &user);

                void ActiveAllPlayers();
                void AddPlayer(const PlayerUserPtr &user);
                void SetPublisher(UserPtr &user);

                StreamPtr GetStream();
                const string & SessionName() const;
                void SetAppInfo(AppInfoPtr &ptr);
                AppInfoPtr GetAppInfo();
                bool IsPublishing() const;
                void Clear();
            private:
                void CloseUserNoLock(const UserPtr &user);

                std::string session_name_;
                StreamPtr stream_;
                std::atomic<int64_t> player_left_time_; 
                UserPtr publisher_;
                AppInfoPtr appinfo_;
                std::mutex lock_;
                std::unordered_set<PlayerUserPtr> play_users_;
        };

    }

}
