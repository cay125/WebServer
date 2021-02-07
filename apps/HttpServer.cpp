//
// Created by xiangpu on 20-3-8.
//
#include <iostream>

#include "apps/HttpServer.hpp"

Fire::App::HttpServer::HttpServer(EventLoop *loop, uint16_t port, std::string _root_dir, int threadNum) : server(loop, port, threadNum),
                                                                                                          root_dir(std::move(_root_dir)),
                                                                                                          event_loop(loop), timer_queue(loop)
{
    server.setConnectionCallback(std::bind(&HttpServer::HandleConnect, this, std::placeholders::_1));
}

void Fire::App::HttpServer::RegisterHandler(std::string url, std::function<void(std::shared_ptr<Fire::TcpConnection>, Fire::App::httpRequest)> &&callback)
{
    url2cb[url] = std::move(callback);
}

void Fire::App::HttpServer::HandleConnect(std::shared_ptr<Fire::TcpConnection> conn)
{
    if (conn->connectionState() == TcpConnection::STATE::connected)
    {
        std::shared_ptr<HttpData> httpUnit(new HttpData(&url2cb, &timer_queue, root_dir));
        conn->setMessageCallback(std::bind(&HttpData::HandleRead, httpUnit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        //conn->setWriteCallback(std::bind(&HttpData::HandleWriteFinish, httpUnit, std::placeholders::_1));
        event_loop->runInLoop([this, httpUnit, conn]()
                              { Conn2Http[conn] = httpUnit; });
    }
    else
    {
        event_loop->runInLoop([this, conn]()
                              { Conn2Http.erase(conn); });
    }
}

void Fire::App::HttpServer::Start()
{
    server.start();
}

