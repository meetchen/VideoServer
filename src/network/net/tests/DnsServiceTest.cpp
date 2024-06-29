/**
 * @FilePath     : /VideoServer/src/network/net/tests/DnsServiceTest.cpp
 * @Description  :  Test DnsService
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 00:33:57
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/


#include "network/DnsService.h"
#include <iostream>

using namespace vdse::network;

int main(int argc, char const *argv[])
{
    std::vector<InetAddressPtr> list;
    sDnsService->AddHost("www.baidu.com");
    sDnsService->Start();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    list = sDnsService->GetHostAddress("www.baidu.com");
    for (auto& addr : list)
    {   
        // TODO FIX IPV6 parse
        std::cout << addr->IP() << std::endl;
    }
    return 0;
}
