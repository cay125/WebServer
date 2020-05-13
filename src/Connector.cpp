//
// Created by xiangpu on 20-5-12.
//
#include "Connector.hpp"
#include <iostream>

Fire::Connector::Connector(Fire::eventLoop *_loop, Fire::netAddr _addr) : event_loop(_loop), peerAddr(_addr), state(CONN_STATE::disconnected), queue(_loop)
{

}

void Fire::Connector::Start()
{
    event_loop->runInLoop(std::bind(&Connector::Connect, this));
}

void Fire::Connector::Restart()
{

}

void Fire::Connector::Stop()
{
    if (state == connected)
    {
        event_loop->runInLoop([this]()
                              {
                                  conn_channel->remove();
                                  conn_channel->clearCallback();
                                  conn_channel.reset();
                                  conn_sock->close();
                                  conn_sock.reset();
                                  state = CONN_STATE::disconnected;
                              });
    }
}

void Fire::Connector::Connect()
{
    if (state == CONN_STATE::connected)
        return;
    conn_sock.reset(new Socket(Socket::createSocket()));
    int res = conn_sock->connect(peerAddr);
    int errorCode = (res == 0) ? 0 : errno;
    switch (errorCode)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            Connecting();
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            std::cout << "Connector will retry after 1s\n";
            conn_sock->close();
            Retry();
            break;
        default:
            std::cout << "unexpected error happen when connecting remote host\n";
            conn_sock->close();
            break;
    }
}

void Fire::Connector::Retry()
{
    queue.addTimer([this]()
                   {
                       std::cout << "Retry to connect\n";
                       Connect();
                   }, std::chrono::seconds(1));
}

void Fire::Connector::Connecting()
{
    state = CONN_STATE::connected;
    conn_channel.reset(new Channel(event_loop, conn_sock->GetSocketFd()));
    conn_channel->setWriteCallback(std::bind(&Connector::HandleWrite, this), true);
}

void Fire::Connector::HandleWrite()
{
    if (state == CONN_STATE::connected && newConnCallback)
        newConnCallback(conn_sock->GetSocketFd());
}

void Fire::Connector::setNewConnCallback(std::function<void(int)> &&cb)
{
    newConnCallback = std::move(cb);
}

Fire::Connector::CONN_STATE Fire::Connector::GetState()
{
    return state;
}
