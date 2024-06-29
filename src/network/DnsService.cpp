/**
 * @FilePath     : /VideoServer/src/network/DnsService.cpp
 * @Description  :  管理DNS解析，避免短时间内对于一个域名的重复解析请求
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-06-30 00:33:37
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/DnsService.h"
#include "network/base/Network.h"
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cstring>

using namespace vdse::network;

namespace
{
    static InetAddressPtr inet_address_ptr_null;
}


DnsService::~DnsService()
{

}


void DnsService::AddHost(const std::string &host)
{
    std::lock_guard<std::mutex> lk(lock_);

    auto it = hosts_info_.find(host);
    if (it == hosts_info_.end())
    {
        hosts_info_[host] = std::vector<InetAddressPtr>();
    }
}

InetAddressPtr DnsService::GetHostAddress(const std::string &host, int index)
{
    std::lock_guard<std::mutex> lk(lock_);

    auto it = hosts_info_.find(host);
    if (it == hosts_info_.end())
    {
        return InetAddressPtr();
    }
    else
    {
        auto hosts = it->second;
        if (hosts.size() > 0)
        {
            return hosts[index % hosts.size()];
        }
    }
    return inet_address_ptr_null;
}

std::vector<InetAddressPtr> DnsService::GetHostAddress(const std::string &host)
{
    std::lock_guard<std::mutex> lk(lock_);
    
    auto it = hosts_info_.find(host);
    if (it == hosts_info_.end())
    {
        return std::vector<InetAddressPtr>();
    }
    else
    {   
        return it->second;
    }
}

void DnsService::UpdateHost(const std::string &host, std::vector<InetAddressPtr> &list)
{
    std::lock_guard<std::mutex> lk(lock_);
    // swap 交换指针，不执行深拷贝
    hosts_info_[host].swap(list);
}

std::unordered_map<std::string, std::vector<InetAddressPtr>> DnsService::GetHosts()
{
    std::lock_guard<std::mutex> lk(lock_);
    
    return hosts_info_;
}

void DnsService::SetDnsServiceParam(int32_t interval, int32_t sleep, int32_t retry)
{
    std::lock_guard<std::mutex> lk(lock_);

    interval_ = interval;
    sleep_ = sleep;
    retry_ = retry;
}

void DnsService::Start()
{
    running_ = true;
    thread_ = std::thread(std::bind(&DnsService::OnWork, this));
}

void DnsService::Stop()
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void DnsService::OnWork()
{
    while (running_)
    {
        auto host_infos = GetHosts();
        for (auto &host : host_infos)
        {
            for (int i = 0; i < retry_; i++)
            {
                std::vector<InetAddressPtr> list;
                GetHostInfo(host.first, list);
                if (list.size() > 0)
                {
                    UpdateHost(host.first, list);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
    }
}

void DnsService::GetHostInfo(const std::string &host, std::vector<InetAddressPtr> &list)
{
    struct addrinfo ainfo, *res;
    memset(&ainfo, 0x00, sizeof (struct addrinfo));

    // 不指定协议族
    ainfo.ai_family = AF_UNSPEC;

    // 被动的
    ainfo.ai_flags = AI_PASSIVE;

    // 使用UDP包
    ainfo.ai_socktype = SOCK_DGRAM;

    auto ret = ::getaddrinfo(host.c_str(), nullptr, &ainfo, &res);

    if (ret == -1) 
    {
        return;
    }

    
    for (struct addrinfo *rp = res; rp != nullptr; rp = rp->ai_next)
    {
        InetAddressPtr addr_ptr = std::make_shared<InetAddress>();
        if (rp->ai_family == AF_INET)
        {
            char ip[INET_ADDRSTRLEN] = {0};
            // 将原本假定为ipv6的结构体转化为ipv4 ！！！
            struct sockaddr_in *addr = (struct sockaddr_in*)&rp->ai_addr;
            ::inet_ntop(AF_INET, &(addr->sin_addr.s_addr), ip, INET_ADDRSTRLEN);
            addr_ptr->SetAddr(ip);
            addr_ptr->SetPort(ntohs(addr->sin_port));

        }
        else if (rp->ai_family == AF_INET6)
        {
            char ip[INET6_ADDRSTRLEN] = {0};
            struct sockaddr_in6 *addr = (struct sockaddr_in6*)&rp->ai_addr;

            ::inet_ntop(AF_INET6, &(addr->sin6_addr), ip, INET6_ADDRSTRLEN);
            addr_ptr->SetAddr(ip);
            addr_ptr->SetPort(ntohs(addr->sin6_port));
            addr_ptr->SetIsIPV6(true);
        }
        list.push_back(std::move(addr_ptr));
    }

}