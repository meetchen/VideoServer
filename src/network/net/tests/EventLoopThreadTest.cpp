#include "network/net/Event.h"
#include "network/net/TimingWheel.h"
#include "network/net/EventLoopThread.h"
#include "network/net/PipeEvent.h"
#include "network/net/EventLoopThreadPool.h"
#include <iostream>
#include <thread>
#include "base/TTime.h"

using namespace vdse::network;


EventLoopThread eventloop_thread;
std::thread th;

void TestEventLoopThread()
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();
    if (loop)
    {

        PipeEventPtr pipe = std::make_shared<PipeEvent>(loop);

        std::cout << "Loop : " << loop << std::endl;
        loop->AddEvent(pipe);

        th = std::thread([&pipe](){
            while (1)
            {
                int64_t now = vdse::base::TTime::NowMS();
                pipe->Write((const char *)&now, sizeof(now));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
        
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    }
}

void testEventLoopThreadPool()
{
    EventLoopThreadPool pool(2,0,2);
    pool.Start();
    auto loops = pool.GetLoops();

    std::cout << "now thread : pid = " << std::this_thread::get_id() << std::endl;


    // for (auto& l : loops)
    // {
    //     l->AddFunction([&l](){
    //         std::cout << "loop : " << l << "thread : " << std::this_thread::get_id() << std::endl;
    //     });
    // }

    // for (int i = 0; i < pool.Size(); i++)
    // {
    //     std::cout << pool.GetNextLoop() << std::endl;
    // }

    auto pl = pool.GetNextLoop();
    pl->RunAfter(1, [](){
        std::cout << "1s afer, now : "<< vdse::base::TTime::NowMS() << std::endl;
    });

    pl->RunAfter(5, [](){
        std::cout << "5s after, now : "<< vdse::base::TTime::NowMS() << std::endl;
    });

    pl->RunEvery(10, [](){
        std::cout << "10s per, now : "<< vdse::base::TTime::NowMS() << std::endl;
    });

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}

void TestEventLoopRunInLoop()
{
    EventLoopThreadPool pool(2,0,2);
    pool.Start();
    auto loops = pool.GetLoops();

    std::cout << "now thread : pid = " << std::this_thread::get_id() << std::endl;


    for (auto& l : loops)
    {
        l->RunInLoop([&l](){
            std::cout << "loop : " << l << " thread : " << std::this_thread::get_id() << std::endl;
        });
    }

    // for (int i = 0; i < pool.Size(); i++)
    // {
    //     std::cout << pool.GetNextLoop() << std::endl;
    // }

}

int main(int argc, char const *argv[])
{
    // TestEventLoopThread();
    // testEventLoopThreadPool();
    TestEventLoopRunInLoop();
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    
    return 0;
}
