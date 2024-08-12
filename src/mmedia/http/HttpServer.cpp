/*
 * @Author: duanran 995122760@qq.com
 * @Date: 2024-08-12 22:05:32
 * @LastEditors: duanran 995122760@qq.com
 * @LastEditTime: 2024-08-12 22:15:10
 * @FilePath: /VideoServer/src/mmedia/http/HttpServer.cpp
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "mmedia/http/HttpServer.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/http/HttpContext.h"
// #include "mmedia/flv/FlvContext.h"

using namespace vdse::mmedia;
using namespace vdse::network;
using HttpContextPtr = std::shared_ptr<HttpContext>;
// using FlvContextPtr = std::shared_ptr<FlvContext>;
HttpServer::HttpServer(EventLoop *loop,const InetAddress &local,HttpCallBack *handler)
:TcpServer(loop,local),http_handler_(handler)
{

}
HttpServer::~HttpServer()
{
    Stop();
}

void HttpServer::Start()
{
    TcpServer::SetActiveCallBack(std::bind(&HttpServer::OnActive,this,std::placeholders::_1));
    TcpServer::SetDestroyConnectionCallBack(std::bind(&HttpServer::OnDestroyed,this,std::placeholders::_1));
    TcpServer::SetNewConnecitonCallBack(std::bind(&HttpServer::OnNewConnection,this,std::placeholders::_1));
    TcpServer::SetWriteCompleteCallBack(std::bind(&HttpServer::OnWriteComplete,this,std::placeholders::_1));
    TcpServer::SetMsgCallBack(std::bind(&HttpServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
    TcpServer::Start();
    HTTP_DEBUG << "HttpServer Start";
}
void HttpServer::Stop()
{
    TcpServer::Stop();
}

void HttpServer::OnNewConnection(const TcpConnectionPtr &conn)
{
    if(http_handler_)
    {
        http_handler_->OnNewConnection(conn);
    }
    HttpContextPtr shake = std::make_shared<HttpContext>(conn,http_handler_);
    conn->SetContext(kHttpContext,shake);
}
void HttpServer::OnDestroyed(const TcpConnectionPtr &conn)
{
    if(http_handler_)
    {
        http_handler_->OnConnectionDestroy(conn);
    }
    conn->ClearContext(kHttpContext);
}
void HttpServer::OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf)
{
    HttpContextPtr shake = conn->GetContext<HttpContext>(kHttpContext);
    if(shake)
    {
        int ret = shake->Parse(buf);
        if(ret == -1)
        {
            conn->ForceClose();
        }
    }
}
void HttpServer::OnWriteComplete(const ConnectionPtr &conn)
{
    HttpContextPtr shake = conn->GetContext<HttpContext>(kHttpContext);
    if(shake)
    {
        shake->WriteComplete(std::dynamic_pointer_cast<TcpConnection>(conn));
    }
    // FlvContextPtr flv = conn->GetContext<FlvContext>(kFlvContext);
    // if(flv)
    // {
    //     flv->WriteComplete(std::dynamic_pointer_cast<TcpConnection>(conn));
    // }    
}
void HttpServer::OnActive(const ConnectionPtr &conn)
{
    if(http_handler_)
    {
        http_handler_->OnActive(conn);
    }
}