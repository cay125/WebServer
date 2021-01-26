//
// Created by xiangpu on 20-5-12.
//

#ifndef FIRESERVER_CONNECTOR_H
#define FIRESERVER_CONNECTOR_H

#include "net/Acceptor.hpp"
#include "net/TimerQueue.hpp"

namespace Fire
{
    class Connector : public std::enable_shared_from_this<Connector>
    {
    public:
        enum CONN_STATE
        {
            connected, disconnected
        };

        Connector(EventLoop *_loop, NetAddr _addr);

        Connector(Connector &) = delete;

        Connector &operator=(Connector &) = delete;

        void Start();

        void Restart();

        void Stop();

        void Connect();

        Connector::CONN_STATE GetState();

        void setNewConnCallback(std::function<void(int)> &&cb);

    private:
        void Retry();

        void Connecting();

        void HandleWrite();

        std::function<void(int)> newConnCallback;
        EventLoop *event_loop;
        NetAddr peerAddr;
        std::unique_ptr<Socket> conn_sock;
        std::unique_ptr<Channel> conn_channel;
        CONN_STATE state;
        TimerQueue queue;
    };
}

#endif //FIRESERVER_CONNECTOR_H
