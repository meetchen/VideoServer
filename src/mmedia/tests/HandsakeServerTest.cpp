/**
 * @FilePath     : /VideoServer/src/mmedia/tests/HandsakeServerTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-01 23:57:59
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "mmedia/rtmp/RtmpHandshake.h"
#include "network/TcpServer.h"
#include <memory>
#include <iostream>

using namespace vdse::network;
using namespace vdse::mmedia;

EventLoopThread eventloop_thread;
std::thread th;

using RtmpHandshakePtr = std::shared_ptr<RtmpHandshake>;

const char *http_resp = "HTTP/1.0 200 OK\r\nDate:Mon,31Dec200104:25:57GMT\r\nServer:Apache/1.3.14(Unix) \r\nContent-type:text/html \r\nLast-modified:Tue,17Apr200106:46:28GMT \r\n Content-length:0 \r\n\r\n" ;

int main(int argc, char const *argv[])
{
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress listen("192.168.159.131:1935");
        TcpServer server(loop, listen);

        // 设置接受到消息时候的回调
        server.SetMsgCallBack([](const TcpConnectionPtr &conn, MsgBuffer &buf){
            RtmpHandshakePtr shake = std::make_shared<RtmpHandshake>(conn, false);
            shake->Handshake(buf);
            conn -> Send(http_resp, strlen(http_resp));
        });

        // 设置检测到新链接时候的回调
        server.SetNewConnecitonCallBack([&loop](const TcpConnectionPtr &conn){
            RtmpHandshakePtr shake = std::make_shared<RtmpHandshake>(conn, false);
            shake->Start();
            conn -> SetWriteCompleteCallBack([&loop](const TcpConnectionPtr &conn){
                RtmpHandshakePtr shake = std::make_shared<RtmpHandshake>(conn, false);
                shake->WriteComplete();
            });
        });

        server.Start();
        
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}
