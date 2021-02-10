//
// Created by xiangpu on 20-3-8.
//

#ifndef FIRESERVER_HTTPDATA_HPP
#define FIRESERVER_HTTPDATA_HPP

#include <filesystem>

#include "net/TcpServer.hpp"
#include "net/TimerQueue.hpp"

namespace fs=std::filesystem;

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
            enum class Version
            {
                vunknown, v11, v10
            };
            enum class Method
            {
                GET, POST, PUT, HEAD
            };
            Method method;
            Version version;
            std::string file_name;
            std::string query;
            std::string body;
            std::string original_url;
            std::map<std::string, std::string> headers;
            std::string to_string();
        };

        class httpResponse
        {
        public:
            enum class StatusCode : int
            {
                c200ok = 200, c404NotFound = 404, c400BadRequest = 400, c301moved = 301
            };
            StatusCode status_code;
            std::string status_msg;
            std::string body;
            std::map<std::string, std::string> headers;

            void makeString(std::string &msg);

            void generateErrorString(std::string &msg, const StatusCode &_status_code, const std::string &_status_msg);
        };


        class HttpData
        {
            enum class STATUS
            {
                STATUS_SUCCESS, STATUS_ERROR
            };
            enum class ParserStatus
            {
                parser_request, parser_head, parser_body, parser_analyse, parser_finish
            };
        public:
            HttpData(HttpData &) = delete;

            HttpData &operator=(HttpData &) = delete;

            explicit HttpData(std::unordered_map<std::string, std::function<void(std::shared_ptr<Fire::TcpConnection>, Fire::App::httpRequest)>> *_url2cb, TimerQueue *_timer_queue, std::string _root_dir);

            void HandleRead(std::shared_ptr<Fire::TcpConnection> p, const char *buf, ssize_t len);

            void HandleWriteFinish(std::shared_ptr<Fire::TcpConnection> p);

        private:

            STATUS parserHttpMsg(std::string msg);

            STATUS parserRequest(std::string requestLine);

            STATUS parserHeader(std::string headLine);

            STATUS analyseRequest();

            void reset();

            httpRequest request;
            httpResponse response;
            fs::path root_dir;
            const int DEFAULT_ALIVE_TIME = 2 * 60 * 1000;
            bool keepAlive;
            TimerQueue *timer_queue;
            bool timer_start = false;
            std::unordered_map<std::string, std::function<void(std::shared_ptr<Fire::TcpConnection>, Fire::App::httpRequest)>> *url2cb;
        };
    }

}
#endif //FIRESERVER_HTTPDATA_HPP
