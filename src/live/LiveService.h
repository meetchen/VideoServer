/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-07-16 10:21:45
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 18:58:28
 * @FilePath: /VideoServer/src/live/LiveService.h
 * @Description: 直播业务的管理类
 * 
 * Copyright (c) 2024 by duanran, All Rights Reserved. 
 */
#pragma once

#include "network/net/EventLoopThreadPool.h"
#include "network/net/Connection.h"
#include "network/TcpServer.h"
#include "mmedia/rtmp/RtmpCallBack.h"
#include "live/Session.h"
#include <mutex>
#include <vector>
#include <unordered_map>
#include <string>
#include "base/TaskMg.h"
#include "base/NoCopyable.h"
#include "mmedia/base/MMediaCallBack.h"
#include "base/Singleton.h"



namespace vdse
{
    namespace live
    {
        class Session;
        using SessionPtr = std::shared_ptr<Session>;
        using namespace vdse::network;
        using namespace vdse::base;

        class LiveService : public vdse::mmedia::RtmpCallBack
        {
            public:
                LiveService() = default;
                ~LiveService() = default;
                
                SessionPtr CreateSession(const std::string& session_name);
                SessionPtr FindSession(const std::string&  session_name);
                bool CloseSession(const std::string&  session_name);
                void OnTimer(const TaskPtr &t);

                void OnNewConnection(const TcpConnectionPtr &conn) override;
                void OnConnectionDestroy(const TcpConnectionPtr &conn) override;
                void OnActive(const ConnectionPtr &conn) override;
                void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) override;
                void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override;
                bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override;
                bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override;

                void Start();
                void Stop();
                EventLoop* GetNextLoop();

            private:
                EventLoopThreadPool *pool_{nullptr};
                std::vector<TcpServer *> servers_;
                std::mutex lock_;
                std::unordered_map<std::string, SessionPtr> sessions_;
        };
        #define sLiveService vdse::base::Singletion<vdse::live::LiveService>::Instance()

    }
}

