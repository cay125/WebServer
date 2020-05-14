//
// Created by xiangpu on 20-5-13.
//

#ifndef FIRESERVER_TCPCLIENT_H
#define FIRESERVER_TCPCLIENT_H

#include "Connector.hpp"
#include "TcpServer.hpp"

namespace Fire
{
    class TcpClient
    {
    public:
        TcpClient(TcpClient &) = delete;

        TcpClient &operator=(TcpClient &) = delete;

        TcpClient(eventLoop *loop, netAddr remoteAddr);

        void Connect();

        void Disconnect();

        void setNewConnCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&_connectionCallback);

        void setMessageCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> &&_messageCallback);

    private:
        void HandleNewConnection(int fd);

        void HandleClose(std::shared_ptr<TcpConnection> conn);

        eventLoop *event_loop;
        Connector connUtil;
        netAddr clientAddr;
        std::shared_ptr<Fire::TcpConnection> tcpConn;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> connectionCallback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> writeCalback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> closeCallback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> messageCallback;
    };
}


#endif //FIRESERVER_TCPCLIENT_H
