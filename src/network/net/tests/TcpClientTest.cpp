#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"
#include <vector>
#include <iostream>    

using namespace vdse::network;

EventLoopThread eventloop_thread;
std::thread th;

const char *http_req="GET / HTTP/1.0\r\nHost: 192.168.159.131\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
const char *http_resp="HTTP/1.0 200 OK\r\nServer: vdse\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

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
                auto size = htonl(strlen(http_req));
                conn->Send((const char *)&size, sizeof(size));
                conn->Send(http_req, strlen(http_req));
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
