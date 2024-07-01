/**
 * @FilePath     : /VideoServer/src/network/TestContext.cpp
 * @Description  :  
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-02 00:13:24
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#include "network/TestContext.h"
#include <iostream>


using namespace vdse::network;

TestContext::TestContext(const TcpConnectionPtr &conn)
:connection_(conn)
{

}

int TestContext::ParseMessage(MsgBuffer &buf)
{
    while(buf.ReadableBytes() > 0)
    {
        if (state_ == kTestContextHeader)
        {
            if (buf.ReadableBytes() >= 4)
            {
                message_length_ = buf.ReadInt32();
                std::cout << "message_length_ : " << message_length_ << std::endl;
                state_ = kTestContextBody;
                continue;
            }
            return 1;
        }
        else if (state_ == kTestContextBody)
        {
            if (buf.ReadableBytes() >= message_length_)
            {
                std::string tmp;
                tmp.assign(buf.Peek(), message_length_);
                message_body_.append(tmp);
                buf.Retrieve(message_length_);
                message_length_ = 0;
                if (cb_)
                {
                    cb_(connection_, message_body_);
                    message_body_.clear();
                }
                state_ = kTestContextHeader;
            }
        }
    }
    return 1;
}
void TestContext::SetTestMessageCallBack(const TestMessageCallBack&cb)
{
    cb_ = cb;
}
void TestContext::SetTestMessageCallBack(TestMessageCallBack&&cb)
{
    cb_ = std::move(cb);
}