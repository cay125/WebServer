//
// Created by xiangpu on 20-3-2.
//

#ifndef FIRESERVER_ACCEPTOR_HPP
#define FIRESERVER_ACCEPTOR_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

#include "net/EventLoop.hpp"
#include "net/Channel.hpp"

namespace Fire
{
    class NetAddr
    {
    public:
        NetAddr(std::string _ipAddr, uint16_t _port);

        explicit NetAddr(sockaddr_in &_addr);

        uint16_t GetPort();

        std::string GetIpString();

        uint16_t GetPortNetOrder();

        uint32_t GteIpNetOrder();

        std::string GetUrlString();

        void GetSockAddr(sockaddr_in &_addr);

        static const std::string ANY_ADDR;

    private:
        uint16_t net_port = 0;
        uint32_t net_addr = 0;
    };


    class Socket
    {
    public:
        explicit Socket(int fd);

        int acceptOneConn(NetAddr &_addr);

        int GetSocketFd();

        int listening();

        int connect(NetAddr &_addr);

        void close();

        int setBindAddr(NetAddr &_addr);

        static int createSocket();

        static void closeSock(int fd);

    private:
        int sock_fd;
    };

    class Acceptor
    {
        typedef std::function<void(int, NetAddr)> newConnFcn;
    public:
        Acceptor(Acceptor &) = delete;

        Acceptor &operator=(Acceptor &) = delete;

        explicit Acceptor(EventLoop *loop, NetAddr addr);

        void setNewConnCallback(newConnFcn &&fcn);

        void listening();

    private:
        void HandleRead();

        newConnFcn newConnCallback;
        EventLoop *event_loop;
        Socket acceptor_sock;
        Channel acceptor_channel;
    };
}

#endif //FIRESERVER_ACCEPTOR_HPP
