/**
 * @FilePath     : /VideoServer/src/network/net/tests/UdpClientTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 17:13:14
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/UdpClient.h"
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
        InetAddress server("192.168.159.131:34444");
        auto client = std::make_shared<UdpClient>(loop, server);

        client->SetRecvMsgCallBack([](const InetAddress & addr, MsgBuffer buf){
            std::cout << "host : " << addr.ToIpPort() <<  std::endl  
                        << " msg :" << buf.Peek() 
                        << " ----------- " << std::endl;
            buf.RetrieveAll();
        });

        client->SetCloseConnectionCallBack([](const UdpSocketPtr &conn){
            if(conn)
            {
                std::cout << "host:" << conn->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        client->SetWriteCompleteCallBack([](const UdpSocketPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete. " << std::endl;
            }
        });
        client->SetConnectedCallBack([&client](const UdpSocketPtr& conn, bool connected){
            if (connected)
            {
                std::cout << "connected : " << conn->PeerAddr().ToIpPort() << std::endl;

                client->Send("11111", strlen("11111"));
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
    TestUdpWork();
    // TestConnTimeOut();
    return 0;
}
