//
// Created by xiangpu on 20-5-13.
//

#include "net/TcpClient.hpp"

Fire::TcpClient::TcpClient(Fire::EventLoop *loop, NetAddr remoteAddr) : event_loop(loop), connUtil(loop, remoteAddr), clientAddr(remoteAddr)
{
    connUtil.setNewConnCallback(std::bind(&TcpClient::HandleNewConnection, this, std::placeholders::_1));
}

void Fire::TcpClient::HandleNewConnection(int fd)
{
    std::shared_ptr<TcpConnection> newConn(new TcpConnection(event_loop, fd, clientAddr));
    newConn->setMessageCallback(std::move(messageCallback));
    newConn->setConnectionCallback(std::move(connectionCallback));
    newConn->setCloseCallback(std::bind(&TcpClient::HandleClose, this, std::placeholders::_1));
    newConn->Established();
    tcpConn = newConn;
}

void Fire::TcpClient::setNewConnCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&_connectionCallback)
{
    connectionCallback = std::move(_connectionCallback);
}

void Fire::TcpClient::HandleClose(std::shared_ptr<Fire::TcpConnection> conn)
{
    tcpConn.reset();
}

void Fire::TcpClient::setMessageCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> &&_messageCallback)
{
    messageCallback = std::move(_messageCallback);
}

void Fire::TcpClient::Connect()
{
    connUtil.Connect();
}

void Fire::TcpClient::Disconnect()
{
    tcpConn->Shutdown();
    connUtil.Stop();
}
