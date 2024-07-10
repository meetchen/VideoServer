/**
 * @FilePath     : /VideoServer/src/mmedia/tests/RtmpClientTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-07 14:21:33
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"
#include <vector>
#include <iostream>    
#include "mmedia/rtmp/RtmpClient.h"
#include "live/CodecHeader.h"
#include "live/base/CodecUtils.h"

using namespace vdse::network;
using namespace vdse::mmedia;
using namespace vdse::live; 


EventLoopThread eventloop_thread;
std::thread th;

CodecHeader codec_header; 

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
            // std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
            if (CodecUtils::IsCodecHeader(data))
            {
                codec_header.ParseCodecHeader(data);
            }
        }

        void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override
        {
            // std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
            if (CodecUtils::IsCodecHeader(data))
            {
                codec_header.ParseCodecHeader(data);
            }
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
    
    eventloop_thread.Run(); 

    EventLoop *loop = eventloop_thread.Loop();
 

    if (loop)
    {
        RtmpClient client(loop, new RtmpCallBackImp());
        client.Play("rtmp://125.74.18.5/live/liteavdemoplayerstreamid");

        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}


int main(int argc, char const *argv[])
{
    TestTcpWork();
    // TestConnTimeOut();
    return 0;
}
