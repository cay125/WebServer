//
// Created by xiangpu on 20-3-2.
//

#ifndef FIRESERVER_ACCEPTOR_HPP
#define FIRESERVER_ACCEPTOR_HPP

#include "eventLoop.hpp"
#include "Channel.hpp"
#include <sys/socket.h>
#include <string>
#include <netinet/in.h>

namespace Fire
{
    class netAddr
    {
    public:
        netAddr(std::string _ipAddr, uint16_t _port);

        explicit netAddr(sockaddr_in &_addr);

        uint16_t GetPort();

        std::string GetAddr();

        uint16_t GetNetPort();

        uint32_t GteNetAddr();

        void GetSockAddr(sockaddr_in &_addr);

        static const std::string ANY_ADDR;

    private:
        uint16_t net_port;
        uint32_t net_addr;
    };


    class Socket
    {
    public:
        explicit Socket(int fd);

        int acceptOneConn(netAddr &_addr);

        int GetSocketFd();

        int listening();

        int connect(netAddr &_addr);

        void close();

        int setBindAddr(netAddr &_addr);

        static int createSocket();

        static void closeSock(int fd);

    private:
        int sock_fd;
    };

    class Acceptor
    {
        typedef std::function<void(int, netAddr)> newConnFcn;
    public:
        Acceptor(Acceptor &) = delete;

        Acceptor &operator=(Acceptor &) = delete;

        explicit Acceptor(eventLoop *loop, netAddr addr);

        void setNewConnCallback(newConnFcn &&fcn);

        void listening();

    private:
        void HandleRead();

        newConnFcn newConnCallback;
        eventLoop *event_loop;
        Socket acceptor_sock;
        Channel acceptor_channel;
    };
}

#endif //FIRESERVER_ACCEPTOR_HPP
