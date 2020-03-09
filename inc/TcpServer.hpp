//
// Created by xiangpu on 20-3-2.
//

#ifndef FIRESERVER_TCPSERVER_HPP
#define FIRESERVER_TCPSERVER_HPP

#include "eventLoop.hpp"
#include "eventThreadPool.hpp"
#include "Acceptor.hpp"
#include "Buffer.hpp"
#include <set>

namespace Fire
{

    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        enum STATE
        {
            connected, closed
        };

        TcpConnection(TcpConnection &) = delete;

        TcpConnection &operator=(TcpConnection &) = delete;

        TcpConnection(eventLoop *loop, int fd, netAddr _addr);

        void send(std::string msg);

        void HandleRead();

        void HandleWrite();

        void HandleClose();

        void HandleError();

        void Established();

        STATE connectionState();

        void setConnectionCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb);

        void setCloseCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb);

        void setMessageCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> &&cb);

    private:
        STATE state;
        eventLoop *event_loop;
        Channel connChannel;
        netAddr clientAddr;
        Buffer inputBuffer;
        Buffer outputBuffer;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> connectionCallback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> closeCallback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> messageCallback;

        void connDestroyed();
    };

    typedef std::function<void(std::shared_ptr<Fire::TcpConnection>)> connFcn;
    typedef std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> msgFcn;

    class TcpServer
    {
    public:
        explicit TcpServer(eventLoop *loop, uint16_t port, int thread_num = 4);

        void newConnection(int fd, netAddr addr);

        void start();

        void setConnectionCallback(connFcn &&cb);

        void setMessageCallback(msgFcn &&cb);

    private:
        std::set<std::shared_ptr<Fire::TcpConnection>> connSet;
        eventLoopThreadPool thread_pool;
        eventLoop *event_loop;
        Acceptor TcpAcceptor;
        connFcn connectionCallback;
        msgFcn messageCallback;

        void removeConnection(std::shared_ptr<TcpConnection> conn);
    };
}

#endif //FIRESERVER_TCPSERVER_HPP
