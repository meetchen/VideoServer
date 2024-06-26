/**
 * @FilePath     : /VideoServer/src/network/net/tests/TcpServerTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-02 00:14:53
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpServer.h"
#include "network/TestContext.h"

#include <iostream>

using namespace vdse::network;

EventLoopThread eventloop_thread;
std::thread th;
const char *http_resp = "HTTP/1.0 200 OK\r\nDate:Mon,31Dec200104:25:57GMT\r\nServer:Apache/1.3.14(Unix) \r\nContent-type:text/html \r\nLast-modified:Tue,17Apr200106:46:28GMT \r\n Content-length:0 \r\n\r\n" ;



int main(int argc, char const *argv[])
{
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress listen("192.168.159.131:34444");
        TcpServer server(loop, listen);

        // 设置接受到消息时候的回调
        server.SetMsgCallBack([](const TcpConnectionPtr &conn, MsgBuffer &buf){
            TestContextPtr context = conn->GetContext<TestContext>(kNormalContext);
            context->ParseMessage(buf);
        });

        // 设置检测到新链接时候的回调
        server.SetNewConnecitonCallBack([&loop](const TcpConnectionPtr &conn){
            TestContextPtr context = std::make_shared<TestContext>(conn);

            context->SetTestMessageCallBack([](const TcpConnectionPtr&conn, const std::string msg){
                std::cout << "messgae : \n" << msg << std::endl;
            });

            conn->SetContext(kNormalContext, context);
            conn -> SetWriteCompleteCallBack([&loop](const TcpConnectionPtr &conn){
                std::cout << "write complete host " << conn->PeerAddr().ToIpPort() << std::endl;
                // conn->ForceClose();
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
