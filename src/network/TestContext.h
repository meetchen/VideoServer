/**
 * @FilePath     : /VideoServer/src/network/TestContext.h
 * @Description  :  保存Connection时，有限状态机的上下文
 * @Author       : Duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-07-02 00:01:04
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/
#pragma once

#include "network/net/TcpConnection.h"
#include <string>
#include <functional>
#include <memory>

namespace vdse
{
    namespace network
    {
        using TestMessageCallBack = std::function<void(const TcpConnectionPtr&, const  std::string &)>;
        class TestContext
        {
            enum
            {
                kTestContextHeader = 1,
                kTestContextBody
            };
            public:
                TestContext(const TcpConnectionPtr &conn);
                ~TestContext() = default;

                int ParseMessage(MsgBuffer &buf);
                void SetTestMessageCallBack(const TestMessageCallBack&cb);
                void SetTestMessageCallBack(TestMessageCallBack&&cb);
            private:
                TcpConnectionPtr connection_;
                int state_{kTestContextHeader};
                int32_t message_length_{0};
                std::string message_body_;
                TestMessageCallBack cb_;
        };

        using TestContextPtr = std::shared_ptr<TestContext>;
    }
}