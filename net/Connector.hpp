//
// Created by xiangpu on 20-5-12.
//

#ifndef FIRESERVER_CONNECTOR_H
#define FIRESERVER_CONNECTOR_H

#include "Acceptor.hpp"
#include "timerQueue.hpp"

namespace Fire
{
    class Connector : public std::enable_shared_from_this<Connector>
    {
    public:
        enum CONN_STATE
        {
            connected, disconnected
        };

        Connector(eventLoop *_loop, netAddr _addr);

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
        eventLoop *event_loop;
        netAddr peerAddr;
        std::unique_ptr<Socket> conn_sock;
        std::unique_ptr<Channel> conn_channel;
        CONN_STATE state;
        timerQueue queue;
    };
}

#endif //FIRESERVER_CONNECTOR_H
