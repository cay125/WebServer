//
// Created by xiangpu on 20-3-8.
//

#ifndef FIRESERVER_HTTPDATA_HPP
#define FIRESERVER_HTTPDATA_HPP

#include "TcpServer.hpp"

namespace Fire
{
    namespace App
    {
        using namespace Fire;

        class httpRequest
        {
        public:
            enum Version
            {
                vunknown, v11, v10
            };
            enum Method
            {
                GET, POST, PUT, HEAD
            };
            Method method;
            Version version;
            std::string file_name;
            std::string query;
            std::map<std::string, std::string> entity;
        };


        class HttpData
        {
            enum STATUS
            {
                STATUS_SUCCESS, STATUS_ERROR
            };
        public:
            HttpData() = default;

            void HandleRead(std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len);

        private:

            STATUS parserHttpMsg(std::string msg);

            STATUS parserRequest(std::string requestLine);

            STATUS parserHeader(std::string headLine);

            httpRequest request;
        };
    }

}
#endif //FIRESERVER_HTTPDATA_HPP
