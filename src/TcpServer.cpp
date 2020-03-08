//
// Created by xiangpu on 20-3-2.
//
#include "TcpServer.hpp"
#include <iostream>
#include <unistd.h>

Fire::TcpServer::TcpServer(eventLoop *loop, uint16_t port, int thread_num) : event_loop(loop), TcpAcceptor(loop, netAddr(netAddr::ANY_ADDR, port)),
                                                                             thread_pool(loop, thread_num)
{
    std::cout << "server is listening on port: " << port << "\n";
    thread_pool.Start();
    TcpAcceptor.setNewConnCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

void Fire::TcpServer::start()
{
    TcpAcceptor.listening();
}

void Fire::TcpServer::setConnectionCallback(connFcn &&cb)
{
    connectionCallback = cb;
}

void Fire::TcpServer::setMessageCallback(msgFcn &&cb)
{
    messageCallback = cb;
}

void Fire::TcpServer::newConnection(int fd, Fire::netAddr addr)
{
    std::cout << "one connection established from Ip: " << addr.GetAddr() << " Port: " << addr.GetPort() << "\n";
    eventLoop *pool = thread_pool.GetNextThread();
    std::shared_ptr<TcpConnection> conn(new TcpConnection(pool, fd, addr));
    conn->setConnectionCallback(std::move(connectionCallback));
    conn->setMessageCallback(std::move(messageCallback));
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    connSet.insert(conn);
}

void Fire::TcpServer::removeConnection(std::shared_ptr<Fire::TcpConnection> conn)
{
    event_loop->runInLoop([this, conn]()
                          {
                              auto n = connSet.erase(conn);
                              if (n != 1)
                                  std::cout << "Error: erase closed connection\n";
                          });
}

Fire::TcpConnection::TcpConnection(eventLoop *loop, int fd, netAddr _addr) : event_loop(loop), connChannel(event_loop, fd), clientAddr(_addr)
{
    connChannel.setReadCallback(std::bind(&TcpConnection::HandleRead, this));
    connChannel.setWriteCallback(std::bind(&TcpConnection::HandleWrite, this), false);
    connChannel.setCloseCallback(std::bind(&TcpConnection::HandleClose, this));
}

void Fire::TcpConnection::HandleRead()
{
    char buf[65536] = {0};
    ssize_t n = read(connChannel.GetMonitorFd(), buf, sizeof(buf));
    if (n > 0)
        messageCallback(shared_from_this(), buf, n);
    else if (n == 0)
        HandleClose();
    else if (n < 0)
        HandleError();
}

void Fire::TcpConnection::HandleWrite()
{
    std::cout << "enter write handler fd: " << connChannel.GetMonitorFd() << "\n";
    ssize_t n = write(connChannel.GetMonitorFd(), outputBuffer.StartPoint(), outputBuffer.readableBytes());
    if (n < 0)
    {
        std::cout << "Error: write data failed\n";
        n = 0;
    }
    outputBuffer.Remove(n);
    if (outputBuffer.readableBytes() == 0)
        connChannel.disableWriteCallback();
    else
        std::cout << "Going to write more data: " << outputBuffer.readableBytes() << " bytes\n";
}

void Fire::TcpConnection::HandleClose()
{
    std::cout << "one connection closed from Ip: " << clientAddr.GetAddr() << " Port: " << clientAddr.GetPort() << "\n";
    connDestroyed();
    closeCallback(shared_from_this());
}

void Fire::TcpConnection::HandleError()
{

}

void Fire::TcpConnection::send(std::string msg)
{
    ssize_t n = write(connChannel.GetMonitorFd(), msg.data(), msg.size());
    if (n < 0)
    {
        std::cout << "Error: write data failed\n";
        n = 0;
    }
    if (n < msg.size())
    {
        outputBuffer.Appand(msg.data() + n, msg.size() - n);
        connChannel.enableWriteCallback();
        std::cout << "Going to write more data: " << msg.size() - n << " bytes\n";
    }
}

void Fire::TcpConnection::setConnectionCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb)
{
    connectionCallback = cb;
}

void Fire::TcpConnection::setMessageCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> &&cb)
{
    messageCallback = cb;
}

void Fire::TcpConnection::setCloseCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb)
{
    closeCallback = cb;
}

void Fire::TcpConnection::connDestroyed()
{
    if (connectionCallback)
        connectionCallback(shared_from_this());
    event_loop->removeChannel(&connChannel);
    connChannel.clearCallback();
    close(connChannel.GetMonitorFd());
}