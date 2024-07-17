/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-07-01 22:53:04
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-07-16 16:01:59
 * @FilePath: /VideoServer/src/mmedia/tests/RtmpServerTest.cpp
 * @Description: rtmp 握手实现 测试 
 *               ffmpeg -i ~/Downloads/test.mp4 -c:v copy -c:a copy -f flv rtmp://192.168.159.131:1935/live/test
 *               ffmpeg -i ~/Downloads/test.mp4 -c:v copy -c:a copy -f flv rtmp://duanran.top/live/test
 * 
 *               ffmpeg -i /home/workSpace/video/output.mp4 -c:v copy -c:a copy -f flv rtmp://duanran.top/live/test
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "mmedia/rtmp/RtmpServer.h"
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


    if (loop)
    {
        InetAddress listen("0.0.0.0:1935");
        RtmpServer server(loop, listen);

        server.Start();
        
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}
