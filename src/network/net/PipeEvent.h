#pragma once

#include "Event.h"
#include <memory>

namespace vdse
{
    namespace network
    {
        class PipeEvent : public Event
        {
            public:
                PipeEvent(EventLoop *loop);
                ~PipeEvent();

                void OnRead() override;
                void OnError(const std::string &err_msg) override;
                void OnClose() override;
                void Write(const char* data, size_t len);
            private:
                int write_fd_{-1};

        };
        using PipeEventPtr = std::shared_ptr<PipeEvent>;
    }
}