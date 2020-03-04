//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_EVENTLOOP_HPP
#define FIRESERVER_EVENTLOOP_HPP

#include <thread>
#include "Channel.hpp"
#include "eventMonitor.hpp"

namespace Fire
{
    class eventLoop
    {
    public:
        explicit eventLoop();

        ~eventLoop() = default;

        eventLoop(eventLoop &) = delete;

        eventLoop &operator=(eventLoop &) = delete;

        void loop();

        void updateChannel(Channel *channel);

        void removeChannel(Channel *channel);

        void stopLoop();

    private:
        std::thread::id own_thread_id;
        bool isLooping;
        eventMonitor monitor;
    };
}

#endif //FIRESERVER_EVENTLOOP_HPP
