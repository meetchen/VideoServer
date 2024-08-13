/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-13 12:35:33
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-13 12:52:02
 * @FilePath: /VideoServer/src/mmedia/tests/HttpClientTest.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"
#include "mmedia/http/HttpClient.h"

#include <iostream>

using namespace vdse::network;
using namespace vdse::mmedia;
EventLoopThread eventloop_thread;
std::thread th;

class HttpHandlerImpl:public HttpCallBack
{
public:
    void OnNewConnection(const TcpConnectionPtr &conn) override
    {

    }
    void OnConnectionDestroy(const TcpConnectionPtr &conn)override
    {

    }
    void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data)override
    {
        std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
    }
    void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override
    {
        std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
    }
    void OnActive(const ConnectionPtr &conn) 
    {

    }
    void OnSent(const TcpConnectionPtr &conn)
    {

    }
    bool OnSentNextChunk(const TcpConnectionPtr &conn)
    {
        return false;
    } 
    void OnRequest(const TcpConnectionPtr &conn,const HttpRequestPtr &req,const PacketPtr &packet)
    {
        if(!req->IsRequest())
        {
            std::cout << "code:" << req->GetStatusCode() << std::endl; 
            if(packet)
            {
                std::cout << "body\n" 
                    << packet->Data() << std::endl;
            }
        }
    }
};

int main(int argc,const char ** agrv)
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if(loop)
    {
        HttpClient client(loop,new HttpHandlerImpl());
        client.Get("http://www.baidu.com");
        while(1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    
    return 0;
}