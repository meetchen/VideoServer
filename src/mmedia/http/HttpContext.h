/**
 * @FilePath     : /VideoServer/src/mmedia/http/HttpContext.h
 * @Description  :  
 * @Author       : duanran 995122760@qq.com
 * @Version      : 0.0.1
 * @LastEditTime : 2024-08-04 13:45:46
 * @Copyright    : G AUTOMOBILE RESEARCH INSTITUTE CO.,LTD Copyright (c) 2024.
**/

#pragma once

#include "mmedia/http/HttpParser.h"
#include "mmedia/http/HttpCallBack.h"

#include "mmedia/http/HttpRequest.h"
#include "mmedia/base/Packet.h"
#include <string>

namespace vdse
{
    namespace mmedia
    {
        using namespace vdse::network;
        enum HttpContextPostState
        {
            kHttpContextPostInit,
            kHttpContextPostHttp,
            kHttpContextPostHttpHeader,
            kHttpContextPostHttpBody,
            kHttpContextPostHttpStreamHeader,
            kHttpContextPostHttpStreamChunk,
            kHttpContextPostChunkHeader,
            kHttpContextPostChunkLen,
            kHttpContextPostChunkBody,
            kHttpContextPostChunkEOF
        };

        class HttpContext
        {
        public:
            HttpContext(const TcpConnectionPtr &conn ,HttpCallBack *handler);
            ~HttpContext()=default;

            int32_t Parse(MsgBuffer &buf);
            bool PostRequest(const std::string &header_and_body);
            bool PostRequest(const std::string &header, PacketPtr &packet);
            bool PostRequest(HttpRequestPtr &request);
            bool PostChunkHeader(const std::string &header);
            void PostChunk(PacketPtr &chunk);
            void PostEofChunk();
            bool PostStreamHeader(const std::string &header);
            bool PostStreamChunk(PacketPtr &packet);
            void WriteComplete(const TcpConnectionPtr &);
        private:
            TcpConnectionPtr connection_;
            HttpParser http_parser_;
            std::string header_;
            PacketPtr out_pakcet_;
            HttpContextPostState post_state_{kHttpContextPostInit};
            bool header_sent_;
            HttpCallBack *handler_{nullptr};
        };
    }
}