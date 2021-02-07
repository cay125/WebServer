//
// Created by xiangpu on 20-3-2.
//

#ifndef FIRESERVER_TCPSERVER_HPP
#define FIRESERVER_TCPSERVER_HPP

#include <set>

#include "net/EventLoop.hpp"
#include "net/EventThreadPool.hpp"
#include "net/Acceptor.hpp"
#include "net/Buffer.hpp"

namespace Fire
{

    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        enum class STATE
        {
            connected, closed
        };

        TcpConnection(TcpConnection &) = delete;

        TcpConnection &operator=(TcpConnection &) = delete;

        TcpConnection(EventLoop *loop, int fd, NetAddr _addr);

        void send(const std::string &msg);

        void Established();

        void Shutdown();

        EventLoop *GetLoop();

        STATE connectionState();

        void setWriteCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb); //for data finished

        void setConnectionCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb);

        void setCloseCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>)> &&cb);

        void setMessageCallback(std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> &&cb);

    private:
        void HandleRead();

        void HandleWrite();

        void HandleClose();

        void HandleError();

        STATE state;
        EventLoop *event_loop;
        Channel connChannel;
        NetAddr clientAddr;
        Buffer inputBuffer;
        Buffer outputBuffer;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> connectionCallback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> writeCalback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>)> closeCallback;
        std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> messageCallback;

        void connDestroyed();
    };

    typedef std::function<void(std::shared_ptr<Fire::TcpConnection>)> connFcn;
    typedef std::function<void(std::shared_ptr<Fire::TcpConnection>, const char *, ssize_t)> msgFcn;

    class TcpServer
    {
    public:
        explicit TcpServer(EventLoop *loop, uint16_t port, int thread_num = 4);

        void start();

        void setConnectionCallback(connFcn &&cb);

        void setMessageCallback(msgFcn &&cb);

    private:
        void newConnection(int fd, NetAddr addr);

        std::set<std::shared_ptr<Fire::TcpConnection>> connSet;
        EventLoopThreadPool thread_pool;
        EventLoop *event_loop;
        Acceptor TcpAcceptor;
        connFcn connectionCallback;
        msgFcn messageCallback;

        void removeConnection(std::shared_ptr<TcpConnection> conn);
    };
}

#endif //FIRESERVER_TCPSERVER_HPP
