/**
 * @FilePath     : /VideoServer/src/mmedia/tests/TcpClientTest.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-01 22:52:58
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"
#include <vector>
#include <iostream>    

using namespace vdse::network;

EventLoopThread eventloop_thread;
std::thread th;

const char* http_req = "GET / HTTP/1.0\r\nHost: 192.168.159.131\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";

const char *http_resp = "HTTP/1.0 200 OK\r\nDate:Mon,31Dec200104:25:57GMT\r\nServer:Apache/1.3.14(Unix) \r\nContent-type:text/html \r\nLast-modified:Tue,17Apr200106:46:28GMT \r\n Content-length:0 \r\n\r\n" ;

void TestTcpWork()
{
    std::vector<TcpConnectionPtr> list;
    
    eventloop_thread.Run(); 
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress server("192.168.159.131:34444");
        auto client = std::make_shared<TcpClient>(loop, server);

        client->SetRecvMsgCallBack([](const TcpConnectionPtr & conn, MsgBuffer buf){
            std::cout << "host : " << conn->PeerAddr().ToIpPort() <<  std::endl  
                        << " msg :" << buf.Peek() << std::endl;
            buf.RetrieveAll();
        });

        client->SetCloseCallBack([](const TcpConnectionPtr &conn){
            if(conn)
            {
                std::cout << "host:" << conn->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        client->SetWriteCompleteCallBack([](const TcpConnectionPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete. " << std::endl;
            }
        });
        client->SetConnectCallBack([](const TcpConnectionPtr& conn, bool connected){
            if (connected)
            {
                std::cout << "connected : " << conn->PeerAddr().ToIpPort() << std::endl;

                conn->Send(http_req, strlen(http_resp));
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
