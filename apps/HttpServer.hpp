//
// Created by xiangpu on 20-3-8.
//

#ifndef FIRESERVER_HTTPSERVER_HPP
#define FIRESERVER_HTTPSERVER_HPP

#include "net/TcpServer.hpp"
#include "net/TimerQueue.hpp"
#include "apps/HttpData.hpp"

namespace Fire
{
    namespace App
    {
        using namespace Fire;

        typedef std::unordered_map<std::string, std::function<void(std::shared_ptr<Fire::TcpConnection>, Fire::App::httpRequest)>> UrlToCallbackMap;

        class HttpServer
        {
        public:
            explicit HttpServer(EventLoop *loop, uint16_t port, std::string _root_dir = ".", int threadNum = 4);

            void Start();

            void RegisterHandler(std::string url, std::function<void(std::shared_ptr<Fire::TcpConnection>, Fire::App::httpRequest)> &&callback);

        private:
            void HandleConnect(std::shared_ptr<TcpConnection> conn);

            std::map<std::shared_ptr<TcpConnection>, std::shared_ptr<App::HttpData>> Conn2Http;
            TimerQueue timer_queue;
            TcpServer server;
            std::string root_dir;
            EventLoop *event_loop;
            UrlToCallbackMap url2cb;
        };
    }
}

#endif //FIRESERVER_HTTPDATA_HPP
