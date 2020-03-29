//
// Created by xiangpu on 20-3-8.
//

#ifndef FIRESERVER_HTTPSERVER_HPP
#define FIRESERVER_HTTPSERVER_HPP

#include "TcpServer.hpp"
#include "HttpData.hpp"
#include "timerQueue.hpp"

namespace Fire
{
    namespace App
    {
        using namespace Fire;

        typedef std::unordered_map<std::string, connFcn> UrlToCallbackMap;

        class HttpServer
        {
        public:
            explicit HttpServer(eventLoop *loop, uint16_t port, std::string _root_dir = ".", int threadNum = 4);

            void Start();

            void RegisterHandler(std::string url, connFcn &&callback);

        private:
            void HandleConnect(std::shared_ptr<TcpConnection> conn);

            std::map<std::shared_ptr<TcpConnection>, std::shared_ptr<App::HttpData>> Conn2Http;
            timerQueue timer_queue;
            TcpServer server;
            std::string root_dir;
            eventLoop *event_loop;
            UrlToCallbackMap url2cb;
        };
    }
}

#endif //FIRESERVER_HTTPDATA_HPP
