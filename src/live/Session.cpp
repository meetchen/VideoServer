/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-15 15:24:21
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 14:27:56
 * @FilePath: /VideoServer/src/live/Session.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by user.name, All Rights Reserved. 
 */
#include "live/Session.h"
#include "base/DomainInfo.h"
#include "base/TTime.h"
#include "base/StringUtils.h"
#include "live/base/LiveLog.h"
#include "live/RtmpPlayUser.h"


using namespace vdse::live;

namespace
{
    UserPtr null_user;
}

Session::Session(const std::string& session_name)
:session_name_(session_name)
{
    stream_ = std::make_shared<Stream>(*this,session_name);
    player_left_time_ = vdse::base::TTime::NowMS();
} 


int64_t Session::ReadyTime() const
{
    return stream_->ReadyTime();
}
ino64_t Session::SinceStart() const
{
    return stream_->SinceStart();
}
bool Session::IsTimeOut()
{
    if (stream_->Timeout())
    {
        return true;
    }

    auto idle = vdse::base::TTime::NowMS() - player_left_time_;

    if (play_users_.empty() && idle > appinfo_->stream_idle_time)
    {
        LIVE_TRACE << "time out, idle : " << idle;
        return true;
    }

    return false;

}

UserPtr Session::CreatePublisher(const std::string &session_name, const ConnectionPtr &conn, UserType type,  const std::string &param)
{
    if (session_name != session_name_)
    {
        LIVE_ERROR << "create publish user failed.Invalid session name:" << session_name;
        return null_user;
    }

    auto list = vdse::base::StringUtils::SplitString(session_name, "/");
    if (list.size() != 3)
    {
        LIVE_ERROR << "create publish user failed.Invalid session name:" << session_name;
        return null_user;
    }
    auto user = std::make_shared<User>(conn, stream_, shared_from_this());
    user->SetAppInfo(appinfo_);
    user->SetDomainName(list[0]);
    user->SetParam(param);
    user->SetAppName(list[1]);
    user->SetStreamName(list[2]);
    user->SetUserType(type);
    conn->SetContext(kUserContext, user);
    return user;

}

UserPtr Session::CreatePlayer(const std::string &session_name, const ConnectionPtr &conn, UserType type,  const std::string &param)
{
    if (session_name != session_name_)
    {
        LIVE_ERROR << "create player user failed.Invalid session name:" << session_name;
        return null_user;
    }

    auto list = vdse::base::StringUtils::SplitString(session_name, "/");
    if (list.size() != 3)
    {
        LIVE_ERROR << "create player user failed.Invalid session name:" << session_name;
        return null_user;
    }
    PlayerUserPtr user;
    if (type == UserType::kUserTypePlayerRtmp)
    {
        user = std::make_shared<RtmpPlayUser>(conn, stream_, shared_from_this());
    }
    user->SetAppInfo(appinfo_);
    user->SetDomainName(list[0]);
    user->SetParam(param);
    user->SetAppName(list[1]);
    user->SetStreamName(list[2]);
    user->SetUserType(type);

    conn->SetContext(kUserContext, user);
    
    return user;
}

void Session::CloseUser(const UserPtr &user)
{
    if(!user->destroyed_.exchange(true))      
    {
        {
            std::lock_guard<std::mutex> lk(lock_);
            if(user->GetUserType() <= UserType::kUserTypePlayerWebRTC)
            {
                if(publisher_)
                {
                    LIVE_DEBUG << "remove publisher,session name:" << session_name_
                                << ",user:" << user->UserId()
                                << ",elapsed:" << user->ElapsedTime()
                                << ",ReadyTime:" << ReadyTime()
                                << ",stream time:" << SinceStart();

                    publisher_.reset();
                }
            }
            else 
            {
                LIVE_DEBUG << "remove player,session name:" << session_name_
                            << ",user:" << user->UserId()
                            << ",elapsed:" << user->ElapsedTime()
                            << ",ReadyTime:" << ReadyTime()
                            << ",stream time:" << SinceStart();            
                play_users_.erase(std::dynamic_pointer_cast<PlayerUser>(user));
                player_left_time_ = vdse::base::TTime::NowMS();
            }
        }
        user->Close();
    }
}


void Session::ActiveAllPlayers()
{
    std::lock_guard<std::mutex> lk(lock_);
    for (auto & player : play_users_)
    {
        player->Active();
    }
}

void Session::AddPlayer(const PlayerUserPtr &user)
{
    {
        std::lock_guard<std::mutex> lk(lock_);
        play_users_.emplace(user);
    }
    LIVE_DEBUG << " add player , session name " << session_name_ << " , user :" << user->user_id_;

    if (!publisher_)
    {
        //TODO 回源
    }
    user->Active();
}

void Session::SetPublisher(UserPtr &user)
{
    std::lock_guard<std::mutex> lk(lock_);
    if(user == publisher_)
    {
        return;
    }
    if(publisher_&&!publisher_->destroyed_.exchange(true))
    {
        publisher_->Close();
    }
    publisher_ = user;
}


StreamPtr Session::GetStream()
{
    return stream_;
}
const string & Session::SessionName() const
{
    return session_name_;
}

void Session::SetAppInfo(AppInfoPtr &ptr)
{
    appinfo_ = ptr;
}

AppInfoPtr Session::GetAppInfo()
{
    return appinfo_;
}

bool Session::IsPublishing() const
{
    // 首先是空指针 !publisher_ 为true
    // !!publisher_ 当为空指针返回 false
    return !!publisher_;
}

void Session::Clear()
{
    std::lock_guard<std::mutex> lk(lock_);
    if(publisher_)
    {
        CloseUserNoLock(publisher_);
    }
    for (auto &user : play_users_)
    {
        // 避免死锁
        CloseUserNoLock(std::dynamic_pointer_cast<User>(user));
    }
    play_users_.clear();
}

void Session::CloseUserNoLock(const UserPtr &user)
{
    if(!user->destroyed_.exchange(true))      
    {
        if(user->GetUserType() <= UserType::kUserTypePlayerWebRTC)
        {
            if(publisher_)
            {
                LIVE_DEBUG << "remove publisher,session name:" << session_name_
                            << ",user:" << user->UserId()
                            << ",elapsed:" << user->ElapsedTime()
                            << ",ReadyTime:" << ReadyTime()
                            << ",stream time:" << SinceStart();
                user->Close();
                publisher_.reset();
            }
        }
        else 
        {
            LIVE_DEBUG << "remove player,session name:" << session_name_
                        << ",user:" << user->UserId()
                        << ",elapsed:" << user->ElapsedTime()
                        << ",ReadyTime:" << ReadyTime()
                        << ",stream time:" << SinceStart();            
            play_users_.erase(std::dynamic_pointer_cast<PlayerUser>(user));
            user->Close();
            player_left_time_ = vdse::base::TTime::NowMS();
        }
    }
}
