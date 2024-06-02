#include "TTime.h"
#include "StringUtils.h"
#include "NoCopyable.h"
#include "Singleton.h"
#include "TaskMg.h"
#include "Logger.h"
#include "FileLog.h"
#include "FileMg.h"
#include <thread>
#include "LogStream.h"
#include <iostream>

using namespace vdse::base;

static std::thread t;

void TestTTime()
{
    std::cout << " now:" << vdse::base::TTime::Now() << " time:" << time(NULL) << " now ms:" << vdse::base::TTime::NowMS() <<  "ISO Time" << vdse::base::TTime::ISOTime()<<std::endl;
}

void TestString()
{
    const std::string str = "abcdadcb";
    const std::string str1 = "aaaaaa";
    const std::string str2 = "";
    const std::string str3 = "a";
    const std::string str4 = "aa;ab;ac;ad;ae;";
    const std::string str5 = ";;;;;";
    const std::string start = "abc";
    const std::string start1 = "abca";
    const std::string start2 = "";
    const std::string de = ";";

    std::cout << "start:" << start << " str:" << str << " result:" << vdse::base::StringUtils::StartsWith(str, start) << std::endl;
    std::cout << "start1:" << start1 << " str:" << str << " result:" << vdse::base::StringUtils::StartsWith(str, start1) << std::endl;
    std::cout << "start2:" << start2 << " str:" << str << " result:" << vdse::base::StringUtils::StartsWith(str, start2) << std::endl;
    std::cout << "start:" << start << " str1:" << str1 << " result:" << vdse::base::StringUtils::StartsWith(str1, start) << std::endl;
    std::cout << "start:" << start << " str2:" << str2 << " result:" << vdse::base::StringUtils::StartsWith(str2, start) << std::endl;
    std::cout << "start:" << start << " str3:" << str3 << " result:" << vdse::base::StringUtils::StartsWith(str3, start) << std::endl;

    std::cout << "end:" << start << " str:" << str << " result:" << vdse::base::StringUtils::EndsWith(str, start) << std::endl;
    std::cout << "end1:" << start1 << " str:" << str << " result:" << vdse::base::StringUtils::EndsWith(str, start1) << std::endl;
    std::cout << "end2:" << start2 << " str:" << str << " result:" << vdse::base::StringUtils::EndsWith(str, start2) << std::endl;
    std::cout << "end:" << start << " str1:" << str1 << " result:" << vdse::base::StringUtils::EndsWith(str1, start) << std::endl;
    std::cout << "end:" << start << " str2:" << str2 << " result:" << vdse::base::StringUtils::EndsWith(str2, start) << std::endl;
    std::cout << "end:" << start << " str3:" << str3 << " result:" << vdse::base::StringUtils::EndsWith(str3, start) << std::endl;

    std::vector<std::string> list = vdse::base::StringUtils::SplitString(str4, de);
    std::cout << "delimiter:" << de << " str4:" << str4 << " result:" << list.size() << std::endl;
    for (auto v : list)
    {
        std::cout << v << std::endl;
    }

    list = vdse::base::StringUtils::SplitString(str5, de);
    std::cout << "delimiter:" << de << " str5:" << str5 << " result:" << list.size() << std::endl;
    for (auto v : list)
    {
        std::cout << v << std::endl;
    }

    list = vdse::base::StringUtils::SplitString(str3, de);
    std::cout << "delimiter:" << de << " str3:" << str3 << " result:" << list.size() << std::endl;
    for (auto v : list)
    {
        std::cout << v << std::endl;
    }

    list = vdse::base::StringUtils::SplitString(str2, de);
    std::cout << "delimiter:" << de << " str2:" << str2 << " result:" << list.size() << std::endl;
    for (auto v : list)
    {
        std::cout << v << std::endl;
    }
}

void TestSingle()
{
    class A : public vdse::base::NoCopyable
    {
        public:
            void show()
            {
                std::cout << "this is A !" << std::endl;
            }
    };

    auto sa = vdse::base::Singletion<A>::Instance();

    sa->show();

}

void TestTaskMg()
{
    TaskPtr task1 = std::make_shared<Task>([](const TaskPtr& task){
        std::cout << "task1: " << 1000 << std::endl;
        task->Restart();
    }, 1000);

    TaskPtr task2 = std::make_shared<Task>([](const TaskPtr& task){
        std::cout << "task2: " << 3000 << std::endl;
        task->Restart();
    }, 3000);

    sTm->Add(task1);
    sTm->Add(task2);
    while (1)
    {
        sTm->OnWork();
    }
}

void testLogStream()
{
    t = std::thread([](){
        while (true)
        {
            LOG_INFO << "test info " << TTime::ISOTime();
            LOG_DEBUG << "test debug " << TTime::ISOTime();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }   
    });

}


int main(int argc, const char **argv)
{
    // TestTTime();
    // TestString();
    // TestSingle();
    // TestTaskMg();

    FileLogPtr log = sFileMg->GetFileLog("test.log");

    log->SetRotate(kRotateMinute);

    vdse::base::g_logger = new Logger(log);

    g_logger->SetLogLevel(kTrace);

    TaskPtr task1 = std::make_shared<Task>([](const TaskPtr& task){
        sFileMg->Oncheck();
        task->Restart();
    }, 1000);
    sTm->Add(task1);
    testLogStream();
    while (1)
    {
        sTm->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    }
    testLogStream();

    return 0;
}