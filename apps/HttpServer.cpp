//
// Created by xiangpu on 20-3-8.
//
#include "HttpServer.hpp"
#include <iostream>

Fire::App::HttpServer::HttpServer(eventLoop *loop, uint16_t port, int threadNum) : server(loop, port, threadNum)
{
    server.setConnectionCallback(std::bind(&HttpServer::HandleConnect, this, std::placeholders::_1));
}

void Fire::App::HttpServer::HandleConnect(std::shared_ptr<Fire::TcpConnection> conn)
{
    if (conn->connectionState() == TcpConnection::connected)
    {
        std::shared_ptr<HttpData> httpUnit(new HttpData());
        Conn2Http[conn] = httpUnit;
        conn->setMessageCallback(std::bind(&HttpData::HandleRead, httpUnit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    else
    {
        Conn2Http.erase(conn);
    }
}

void Fire::App::HttpServer::Start()
{
    server.start();
}

