/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 22:52:57
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-02 16:17:47
 * @FilePath: /VideoServer/src/mmedia/tests/HandsakeClientTest.cpp
 * @Description: 测试rtmp客户端的握手协议
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"
#include <vector>
#include <iostream>    
#include "mmedia/rtmp/RtmpHandshake.h"

using namespace vdse::network;
using namespace vdse::mmedia;


EventLoopThread eventloop_thread;
std::thread th;

const char *http_req="GET / HTTP/1.0\r\nHost: 192.168.159.131\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
const char *http_resp="HTTP/1.0 200 OK\r\nServer: vdse\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

void TestTcpWork()
{
    std::vector<TcpConnectionPtr> list;
    
    eventloop_thread.Run(); 

    EventLoop *loop = eventloop_thread.Loop();
    using RtmpHandshakePtr =  std::shared_ptr<RtmpHandshake>;


    if (loop)
    {
        InetAddress server("192.168.159.131:1935");
        auto client = std::make_shared<TcpClient>(loop, server);

        client->SetRecvMsgCallBack([](const TcpConnectionPtr & conn, MsgBuffer buf){
            auto shake = conn -> GetContext<RtmpHandshake>(kNormalContext);
            shake -> Handshake(buf);
        });

        client->SetCloseCallBack([](const TcpConnectionPtr &conn){
            if(conn)
            {
                std::cout << "host:" << conn->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        client->SetWriteCompleteCallBack([](const TcpConnectionPtr &conn){
            if(conn)
            {
                // std::cout << "host:" << conn->PeerAddr().ToIpPort() << " write complete. " << std::endl;
                auto shake = conn -> GetContext<RtmpHandshake>(kNormalContext);
                shake->WriteComplete();
            }
        });
        client->SetConnectCallBack([](const TcpConnectionPtr& conn, bool connected){
            if (connected)
            {
                // auto size = htonl(strlen(http_req));
                // conn->Send((const char *)&size, sizeof(size));
                // conn->Send(http_req, strlen(http_req));
                RtmpHandshakePtr shake = std::make_shared<RtmpHandshake>(conn, true);
                conn -> SetContext(kNormalContext, shake);
                shake -> Start();
            }
        });

        client->Connect();

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
