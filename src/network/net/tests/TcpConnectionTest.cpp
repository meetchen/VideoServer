#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include <vector>
#include <iostream>

using namespace vdse::network;

EventLoopThread eventloop_thread;
std::thread th;

const char *http_resp = "HTTP/1.0 200 OK\r\nDate:Mon,31Dec200104:25:57GMT\r\nServer:Apache/1.3.14(Unix) \r\nContent-type:text/html \r\nLast-modified:Tue,17Apr200106:46:28GMT \r\n Content-length:0 \r\n\r\n" ;

void TestTcpWork()
{
    std::vector<TcpConnectionPtr> list;
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress server("192.168.159.131:34444");
        // 开始接受请求
        auto acceptor = std::make_shared<Acceptor>(loop, server);
        // 设置接受到请求后的回调
        acceptor->SetAcceptCallBack([&loop, &server, &list](int fd, const InetAddress &addr){
            std::cout << "host :" << addr.ToIpPort() << std::endl;
            auto connection = std::make_shared<TcpConnection>(loop, fd, server, addr);

            connection->SetRecvMsgCallBack([](const TcpConnectionPtr &con, MsgBuffer &buf)
            {
                std::cout << "revc msg : " << buf.Peek() << std::endl;
                buf.RetrieveAll();
                con->Send(http_resp, strlen(http_resp));
            });

            connection->SetWriteCompleteCallBack([&loop](const TcpConnectionPtr &con){
                std::cout << "write complete host :" << con->PeerAddr().ToIpPort() << std::endl;
                loop->DelEvent(con);
                con->ForceClose();
            });

            list.push_back(connection);
            loop->AddEvent(connection);
        });
        acceptor->Start();
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void TestConnTimeOut()
{
    std::vector<TcpConnectionPtr> list;
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress server("192.168.159.131:34444");
        // 开始接受请求
        auto acceptor = std::make_shared<Acceptor>(loop, server);
        // 设置接受到请求后的回调
        acceptor->SetAcceptCallBack([&loop, &server, &list](int fd, const InetAddress &addr){
            std::cout << "host :" << addr.ToIpPort() << std::endl;
            auto connection = std::make_shared<TcpConnection>(loop, fd, server, addr);

            connection->SetRecvMsgCallBack([](const TcpConnectionPtr &con, MsgBuffer &buf)
            {
                std::cout << "revc msg : " << buf.Peek() << std::endl;
                buf.RetrieveAll();
                // con->Send(http_resp, strlen(http_resp));
            });

            connection->SetWriteCompleteCallBack([&loop](const TcpConnectionPtr &con){
                std::cout << "write complete host :" << con->PeerAddr().ToIpPort() << std::endl;
                loop->DelEvent(con);
                con->ForceClose();
            });

            connection->SetTimeOutCallBack(2, [](const TcpConnectionPtr &con){
                std::cout << "time out" << std::endl;
            });

            connection->EnableCheckIdleTimeOut(3);

            list.push_back(connection);
            loop->AddEvent(connection);
        });
        acceptor->Start();
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
