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

        class cType
        {
        public:
            static void typeInit();

            static std::string GetType(std::string suffix);

        private:
            static std::map<std::string, std::string> suffix2type;
        };

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
            std::string body;
            std::map<std::string, std::string> headers;
        };

        class httpResponse
        {
        public:
            enum StatusCode
            {
                c200ok = 200, c404NotFound = 404, c400BadRequest = 400, c301moved = 301
            };
            StatusCode status_code;
            std::string status_msg;
            std::string body;
            std::map<std::string, std::string> headers;

            void makeString(std::string &msg);
        };


        class HttpData
        {
            enum STATUS
            {
                STATUS_SUCCESS, STATUS_ERROR
            };
            enum ParserStatus
            {
                parser_request, parser_head, parser_body, parser_analyse, parser_finish
            };
        public:
            HttpData();

            void HandleRead(std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len);

        private:

            STATUS parserHttpMsg(std::string msg);

            STATUS parserRequest(std::string requestLine);

            STATUS parserHeader(std::string headLine);

            STATUS analyseRequest();

            httpRequest request;
            httpResponse response;
            const int DEFAULT_ALIVE_TIME = 10 * 60 * 1000;
        };
    }

}
#endif //FIRESERVER_HTTPDATA_HPP
