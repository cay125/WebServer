//
// Created by xiangpu on 20-3-2.
//
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "net/Acceptor.hpp"

Fire::Acceptor::Acceptor(EventLoop *loop, NetAddr addr) : event_loop(loop), acceptor_sock(Socket::createSocket()),
                                                          acceptor_channel(loop, acceptor_sock.GetSocketFd())
{

    acceptor_sock.setBindAddr(addr);
    //acceptor_sock.listening();
    acceptor_channel.setReadCallback(std::bind(&Acceptor::HandleRead, this));
}

void Fire::Acceptor::listening()
{
    acceptor_sock.listening();
}

void Fire::Acceptor::HandleRead()
{
    NetAddr clientAddr("0.0.0.0", 0);
    int fd = acceptor_sock.acceptOneConn(clientAddr);
    newConnCallback(fd, clientAddr);
}

void Fire::Acceptor::setNewConnCallback(Fire::Acceptor::newConnFcn &&fcn)
{
    newConnCallback = fcn;
}

const std::string Fire::NetAddr::ANY_ADDR = "ANY_ADDR";

Fire::NetAddr::NetAddr(std::string _ipAddr, uint16_t _port)
{
    if (_ipAddr != ANY_ADDR)
        net_addr = inet_addr(_ipAddr.c_str());
    else
        net_addr = htonl(INADDR_ANY);
    net_port = htons(_port);
}

Fire::NetAddr::NetAddr(sockaddr_in &_addr)
{
    net_addr = _addr.sin_addr.s_addr;
    net_port = _addr.sin_port;
}

Fire::Socket::Socket(int fd) : sock_fd(fd)
{
}

void Fire::Socket::closeSock(int fd)
{
    ::close(fd);
}

int Fire::Socket::acceptOneConn(NetAddr &_addr)
{
    sockaddr_in addr;
    socklen_t sock_len = sizeof(addr);
    int client_sock = accept4(sock_fd, (sockaddr *) &addr, &sock_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    _addr = NetAddr(addr);
    return client_sock;
}

int Fire::Socket::listening()
{
    int res = listen(sock_fd, 100);
    if (res < 0)
        std::cout << "Error: listen error\n";
    return res;
}

int Fire::Socket::setBindAddr(Fire::NetAddr &_addr)
{
    sockaddr_in addr;
    _addr.GetSockAddr(addr);
    int res = bind(sock_fd, (sockaddr *) &addr, sizeof(addr));
    if (res < 0)
        std::cout << "Error: binding socket addr\n";
    return res;
}

int Fire::Socket::GetSocketFd()
{
    return sock_fd;
}

int Fire::Socket::createSocket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
        std::cout << "Error: Createing socket\n";
    return sockfd;
}

int Fire::Socket::connect(NetAddr &_addr)
{
    sockaddr_in addr;
    _addr.GetSockAddr(addr);
    int res = ::connect(sock_fd, (sockaddr *) &addr, sizeof(addr));
    if (res < 0)
    {
        std::cout << "Error: Connect remote host\n";
        perror("Reason");
    }
    return res;
}

void Fire::Socket::close()
{
    ::close(sock_fd);
}

void Fire::NetAddr::GetSockAddr(sockaddr_in &_addr)
{
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = GetNetPort();
    _addr.sin_addr.s_addr = GteNetAddr();
}

std::string Fire::NetAddr::GetAddr()
{
    in_addr addr = {0};
    addr.s_addr = net_addr;
    return std::string(inet_ntoa(addr));
}

uint16_t Fire::NetAddr::GetPort()
{
    return ntohs(net_port);
}

uint32_t Fire::NetAddr::GteNetAddr()
{
    return net_addr;
}

uint16_t Fire::NetAddr::GetNetPort()
{
    return net_port;
}
