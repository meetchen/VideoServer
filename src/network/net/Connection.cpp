/*
 * @Author: Duanran 995122760@qq.com
 * @Date: 2024-06-26 15:00:58
 * @LastEditors: Duanran 995122760@qq.com
 * @LastEditTime: 2024-07-10 16:26:16
 * @FilePath: /VideoServer/src/network/net/Connection.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "network/net/Connection.h"
#include "network/base/Network.h"
using namespace vdse::network;


Connection::Connection(EventLoop *loop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr)
:Event(loop, fd),
local_addr_(localAddr),
peer_addr_(peerAddr)
{

}


void Connection::SetLocalAddr(const InetAddress &local)
{
    local_addr_ = local;
}
void Connection::SetPeerAddr(const InetAddress &peer)
{
    peer_addr_ = peer;
}

const InetAddress &Connection::LocalAddr() const
{
    return local_addr_;
}
const  InetAddress &Connection::PeerAddr() const
{
    return peer_addr_;
}

void Connection::SetContext(int type, const ContextPtr &context)
{
    contexts_[type] = context;
}

void Connection::SetContext(int type, ContextPtr &&context)
{
    contexts_[type] = std::move(context);
}

void Connection::ClearContext(int type)
{
    NETWORK_DEBUG << "clear context, type : " << type;
    contexts_[type].reset();
}

void Connection::ClearContext()
{
    contexts_.clear();
}

void Connection::SetActiveCallBack(const ActiveCallBack &cb)
{
    active_cb_ = cb;
}

void Connection::SetActiveCallBack(ActiveCallBack &&cb)
{
    active_cb_ = std::move(cb);
}

void Connection::Active()
{
    if (!active_.load())
    {
        loop_->RunInLoop([this](){
            active_.store(true);
            if (active_cb_)
            {
                active_cb_(std::dynamic_pointer_cast<Connection>(shared_from_this()));
            }
        });
    }
}
void Connection::Deactive()
{   
    if (active_.load())
    {
        active_.store(false);
    }
}
