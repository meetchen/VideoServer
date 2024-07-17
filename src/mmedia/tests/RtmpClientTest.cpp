/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 22:52:57
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-17 12:41:46
 * @FilePath: /VideoServer/src/mmedia/tests/RtmpClientTest.cpp
 * @Description: 测试rtmp客户端的握手协议
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"
#include <vector>
#include <iostream>    
#include "mmedia/rtmp/RtmpClient.h"
#include "base/Config.h"
#include "base/LogStream.h"
#include "base/FileMg.h"
#include "base/TaskMg.h"
#include "live/LiveService.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <memory>
#include <filesystem>

using namespace vdse::base;
using namespace vdse::live;
using namespace vdse::mmedia;


using namespace vdse::network;
using namespace vdse::mmedia;


EventLoopThread eventloop_thread;
std::thread th;

class RtmpCallBackImp : public RtmpCallBack
{ 
    public:
        void OnNewConnection(const TcpConnectionPtr &conn) override
        {

        }
        void OnConnectionDestroy(const TcpConnectionPtr &conn) override
        {

        }
        void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) override
        {
            std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
        }

        void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override
        {
            std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
        }

        void OnActive(const ConnectionPtr &conn) override
        {

        }
        bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override
        {   
            return false;
        }
        bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override
        {
            return false;
        }
        void OnPause(const TcpConnectionPtr &conn, bool pause) override
        {

        }
        void OnSeek(const TcpConnectionPtr &conn, double time) override
        {

        }
        void OnPublishPrepare(const TcpConnectionPtr &conn) override
        {

        }

};

void TestTcpWork()
{
    

}


int main(int argc, char const *argv[])
{
    g_logger = std::make_shared<Logger>(nullptr);

    g_logger->SetLogLevel(kDebug);

    
    eventloop_thread.Run(); 

    EventLoop *loop = eventloop_thread.Loop();
 

    if (loop)
    {
        RtmpClient client(loop, new RtmpCallBackImp());
        client.Play("rtmp://125.74.18.5/live/liteavdemoplayerstreamid");
        std::cout << " rtmp://125.74.18.5/live/liteavdemoplayerstreamid " << std::endl;
        while (1)
        {

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
  

    return 0;
}
