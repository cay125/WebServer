//
// Created by xiangpu on 20-3-2.
//
#include <iostream>
#include <memory>
#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <glog/logging.h>

#include "net/Acceptor.hpp"

Fire::Acceptor::Acceptor(EventLoop *loop, NetAddr addr) : event_loop(loop), acceptor_sock(Socket::createSocket()),
                                                          acceptor_channel(loop, acceptor_sock.GetSocketFd())
{

    acceptor_sock.setBindAddr(addr);
    acceptor_channel.setReadCallback(std::bind(&Acceptor::HandleRead, this));
}

void Fire::Acceptor::listening()
{
    acceptor_sock.listening();
}

void Fire::Acceptor::HandleRead()
{
    bool get_conn = false;
    int fd = 0;
    // loop until no more connections
    do
    {
        NetAddr clientAddr("0.0.0.0", 0);
        fd = acceptor_sock.acceptOneConn(clientAddr);
        if (fd > 0)
        {
            if (newConnCallback)
                newConnCallback(fd, clientAddr);
            else 
                Socket::closeSocket(fd);
            get_conn = true;
        }
    }while (fd > 0);
    if (!get_conn)
    {
        LOG(ERROR) << "ERROR: No connection is established";
    }
}

void Fire::Acceptor::setNewConnCallback(Fire::Acceptor::newConnFcn &&fcn)
{
    newConnCallback = fcn;
}

const std::string Fire::NetAddr::ANY_ADDR = "ANY_ADDR";

Fire::NetAddr::NetAddr(std::string _ipAddr, uint16_t _port)
{
    net_port = htons(_port);
    if (_ipAddr == ANY_ADDR)
    {
        net_addr = htonl(INADDR_ANY);
        return;
    }

    bool is_hostname = false;
    for (size_t i = 0; i < _ipAddr.length(); i++)
    {
        if (std::isalpha(_ipAddr[i]))
        {
            is_hostname = true; //means the input parameter is a hostname
            break;
        }
    }
    addrinfo hint{}; //c++11 feature, value initialize all zero
    hint.ai_family = AF_INET;
    if (!is_hostname)
        hint.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV; //means we don't want to resolve anything
    else 
        hint.ai_flags = AI_ALL;
    
    addrinfo *resolved_addr = nullptr;
    int ret = getaddrinfo(_ipAddr.c_str(), nullptr, &hint, &resolved_addr);
    if (ret != 0)
    {
        LOG(ERROR) << "ERROR: Resolve(" << _ipAddr << ") failed";
        throw std::runtime_error("Resolve(" + _ipAddr + ") failed");
    }
    if (resolved_addr == nullptr)
    {
        LOG(ERROR) << "ERROR: getaddrinfo returned successfully but with no results";
        throw std::runtime_error("getaddrinfo returned successfully but with no results");
    }
    auto addrinfo_deleter = [](addrinfo *const x) { freeaddrinfo(x); };
    std::unique_ptr<addrinfo, decltype(addrinfo_deleter)> wrapped_address(resolved_addr, std::move(addrinfo_deleter));

    net_addr = (reinterpret_cast<sockaddr_in *>(wrapped_address->ai_addr))->sin_addr.s_addr; 
}

Fire::NetAddr::NetAddr(sockaddr_in &_addr)
{
    net_addr = _addr.sin_addr.s_addr;
    net_port = _addr.sin_port;
}

Fire::Socket::Socket(int fd) : sock_fd(fd)
{
    CHECK(fd > 0);
}

void Fire::Socket::closeSocket(int fd)
{
    CHECK(fd > 0);
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
        LOG(ERROR) << "ERROR: listen error. Reason: " << strerror(errno);
    return res;
}

int Fire::Socket::setBindAddr(Fire::NetAddr &_addr)
{
    sockaddr_in addr;
    _addr.GetSockAddr(addr);
    int res = bind(sock_fd, (sockaddr *) &addr, sizeof(addr));
    if (res < 0)
        LOG(ERROR) << "ERROR: binding socket addr. Reason: " << strerror(errno);
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
        LOG(ERROR) << "ERROR: Createing socket. Reason: " << strerror(errno);
    return sockfd;
}

int Fire::Socket::connect(NetAddr &_addr)
{
    sockaddr_in addr;
    _addr.GetSockAddr(addr);
    int res = ::connect(sock_fd, (sockaddr *) &addr, sizeof(addr));
    if (res < 0)
    {
        LOG(ERROR) << "ERROR: Connect remote host. Reason: " << strerror(errno);
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
    _addr.sin_port = GetPortNetOrder();
    _addr.sin_addr.s_addr = GteIpNetOrder();
}

std::string Fire::NetAddr::GetIpString()
{
    in_addr addr = {0};
    addr.s_addr = net_addr;
    return std::string(inet_ntoa(addr));
}

uint16_t Fire::NetAddr::GetPort()
{
    return ntohs(net_port);
}

uint32_t Fire::NetAddr::GteIpNetOrder()
{
    return net_addr;
}

uint16_t Fire::NetAddr::GetPortNetOrder()
{
    return net_port;
}

std::string Fire::NetAddr::GetUrlString()
{
    return GetIpString() + ":" + std::to_string(GetPort());
}
