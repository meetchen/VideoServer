/**
 * @FilePath     : /VideoServer/src/network/net/tests/UdpServerTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 17:50:15
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/UdpServer.cpp"
#include <vector>
#include <iostream>    

using namespace vdse::network;

EventLoopThread eventloop_thread;
std::thread th;


void TestUdpWork()
{
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress listen("192.168.159.131:34444");
        auto server = std::make_shared<UdpServer>(loop, listen);

        server->SetRecvMsgCallBack([&server](const InetAddress & addr, MsgBuffer buf){
            std::cout << "host : " << addr.ToIpPort() <<  std::endl  
                        << " msg :" << buf.Peek() << std::endl;
            struct sockaddr_in6 sock_addr;
            addr.GetSockAddr((struct sockaddr *)&sock_addr);
            server->Send(buf.Peek(), buf.ReadableBytes(), (struct sockaddr *)&sock_addr, sizeof(sock_addr));
            buf.RetrieveAll();
        });

        server->SetCloseConnectionCallBack([](const UdpSocketPtr &conn){
            if(conn)
            {
                std::cout << "host:" << conn->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        server->SetWriteCompleteCallBack([](const UdpSocketPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete. " << std::endl;
            }
        });


        server->Start();

        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}


int main(int argc, char const *argv[])
{
    TestUdpWork();
    // TestConnTimeOut();
    return 0;
}
