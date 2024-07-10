/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-10 16:00:11
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 16:12:27
 * @FilePath: /VideoServer/src/live/User.h
 * @Description: 单个请求的数据与操作的集合 rtmp flv 等用户
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include "network/net/TcpConnection.h"
#include "base/AppInfo.h"
#include <string>
#include <memory>

namespace vdse
{
    namespace live
    {
        using namespace vdse::network;
        using namespace vdse::base;
        using std::string;
        using AppInfoPtr = std::shared_ptr<AppInfo>;
        class Session;
        using SessionPtr = std::shared_ptr<Session>;
        
        // 枚举具有类的作用域属性
        enum class UserType
        {
            kUserTypePublishRtmp = 0,
            kUserTypePublishMpegts ,
            kUserTypePublishPav,
            kUserTypePublishWebRtc ,
            kUserTypePlayerPav ,
            kUserTypePlayerFlv ,
            kUserTypePlayerHls ,
            kUserTypePlayerRtmp ,
            kUserTypePlayerWebRTC ,
            kUserTypeUnknowed = 255,
        };

        enum class UserProtocol
        {
            kUserProtocolHttp = 0,
            kUserProtocolHttps ,
            kUserProtocolQuic ,
            kUserProtocolRtsp ,
            kUserProtocolWebRTC ,
            kUserProtocolUdp ,
            kUserProtocolUnknowed = 255
        };
        class Stream;
        using StreamPtr = std::shared_ptr<Stream>;

        class User:public std::enable_shared_from_this<User>
        {
        public:
            friend class Session;
            explicit User(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s);
            virtual ~User() = default;

            const string &DomainName() const ;
            void SetDomainName(const string &domain);
            const string &AppName() const ;
            void SetAppName(const string &domain);
            const string &StreamName() const ;
            void SetStreamName(const string &domain);
            const string &Param() const ;
            void SetParam(const string &domain);   
            const AppInfoPtr &GetAppInfo()const;
            void SetAppInfo(const AppInfoPtr &info);
            virtual UserType GetUserType() const;
            void SetUserType(UserType t);
            virtual UserProtocol GetUserProtocol() const ;
            void SetUserProtocol(UserProtocol p) ;
            
            void Close();
            ConnectionPtr GetConnection();
            uint64_t ElapsedTime();
            void Active();
            void Deactive();
            const std::string &UserId() const 
            {
                return user_id_;
            }
            SessionPtr GetSession() const 
            {
                return session_;
            }
            StreamPtr GetStream() const 
            {
                return stream_;
            }
        protected:
            ConnectionPtr connection_;
            StreamPtr stream_;
            string domain_name_;
            string app_name_;
            string stream_name_;
            string param_;
            string user_id_;
            AppInfoPtr app_info_;
            int64_t start_timestamp_{0};
            UserType type_{UserType::kUserTypeUnknowed};
            UserProtocol protocol_{UserProtocol::kUserProtocolUnknowed};
            std::atomic_bool destroyed_{false};
            SessionPtr session_;
        };
    }
}
