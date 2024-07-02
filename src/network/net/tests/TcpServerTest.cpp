/**
 * @FilePath     : /VideoServer/src/network/net/tests/TcpServerTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-02 08:33:42
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

const char *http_resp="HTTP/1.0 200 OK\r\nServer: vdse\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

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
            // std::string tmp;
            // tmp.assign(buf.Peek(), buf.ReadableBytes());
            // std::cout << " info : " << tmp << std::endl;
            TestContextPtr context = conn->GetContext<TestContext>(kNormalContext);
            context->ParseMessage(buf);
        });

        // 设置检测到新链接时候的回调
        server.SetNewConnecitonCallBack([&loop](const TcpConnectionPtr &conn){
            
            std::cout << "new connection : " << conn->PeerAddr().ToIpPort() << std::endl;

            TestContextPtr context = std::make_shared<TestContext>(conn);
            conn -> SetContext(kNormalContext, context);

            context->SetTestMessageCallBack([](const TcpConnectionPtr&conn, const std::string msg){
                std::cout << "messgae : \n" << msg << std::endl;
            });

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
