#include "network/net/Event.h"
#include "network/net/EventLoopThread.h"
#include "network/net/PipeEvent.h"
#include <iostream>
#include <thread>

using namespace vdse::network;


EventLoopThread eventloop_thread;

void TestEventLoopThread()
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();
    if (loop)
    {

        PipeEventPtr pipe = std::make_shared<PipeEvent>(loop);

        std::cout << "Loop : " << loop << std::endl;
        loop->AddEvent(pipe);
        int64_t test = 12345;

        pipe->Write((const char *)&test, sizeof(test));
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        
    }
}

int main(int argc, char const *argv[])
{
    TestEventLoopThread();
    return 0;
}
