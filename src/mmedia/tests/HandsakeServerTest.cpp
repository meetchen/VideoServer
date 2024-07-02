/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 22:53:04
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-02 15:48:01
 * @FilePath: /VideoServer/src/mmedia/tests/HandsakeServerTest.cpp
 * @Description: rtmp 握手实现 测试 fmpeg -i ~/Downloads/test.mp4 -c:v copy -c:a copy -f flv rtmp://192.168.159.131:1935/U
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpServer.h"
#include "network/TestContext.h"
#include "mmedia/rtmp/RtmpHandshake.h"

#include <iostream>

using namespace vdse::network;
using namespace vdse::mmedia;

EventLoopThread eventloop_thread;
std::thread th;

const char *http_resp="HTTP/1.0 200 OK\r\nServer: vdse\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

int main(int argc, char const *argv[])
{
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    using RtmpHandshakePtr =  std::shared_ptr<RtmpHandshake>;

    if (loop)
    {
        InetAddress listen("192.168.159.131:1935");
        TcpServer server(loop, listen);

        // 设置接受到消息时候的回调
        server.SetMsgCallBack([](const TcpConnectionPtr &conn, MsgBuffer &buf){
            auto shake = conn -> GetContext<RtmpHandshake>(kNormalContext);
            shake->Handshake(buf);
        });

        // 设置检测到新链接时候的回调
        server.SetNewConnecitonCallBack([&loop](const TcpConnectionPtr &conn){
            
            std::cout << "new connection : " << conn->PeerAddr().ToIpPort() << std::endl;

            RtmpHandshakePtr shake = std::make_shared<RtmpHandshake>(conn, false);
            conn -> SetContext(kNormalContext, shake);
            shake -> Start();

            conn -> SetWriteCompleteCallBack([&loop](const TcpConnectionPtr &conn){
                // std::cout << "write complete host " << conn->PeerAddr().ToIpPort() << std::endl;
                // conn->ForceClose();
                auto shake = conn -> GetContext<RtmpHandshake>(kNormalContext);
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
