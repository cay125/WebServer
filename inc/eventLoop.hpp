//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_EVENTLOOP_HPP
#define FIRESERVER_EVENTLOOP_HPP

#include <thread>
#include <mutex>
#include "Channel.hpp"
#include "eventMonitor.hpp"

namespace Fire
{
    class eventLoop
    {
        typedef std::function<void()> eventCallback;
    public:
        enum STATUS
        {
            STOP, RUNNING, SLEEP
        };

        explicit eventLoop();

        ~eventLoop() = default;

        eventLoop(eventLoop &) = delete;

        eventLoop &operator=(eventLoop &) = delete;

        void loop();

        void updateChannel(Channel *channel);

        void removeChannel(Channel *channel);

        void runInLoop(eventCallback &&cb);

        void queueInLoop(eventCallback &&cb);

        void stopLoop();

        STATUS getStatus()
        { return status; }


    private:
        void doPendingCallback();

        bool isInCurrentThread();

        void HandleRead();

        void wakeSelf();

        std::thread::id own_thread_id;
        STATUS status;
        eventMonitor monitor;
        std::mutex m_mutex;
        int wakeFd;
        Channel wakeChannel;
        std::vector<Fire::eventLoop::eventCallback> pendingCBs;
        bool isDoPendng;
    };
}

#endif //FIRESERVER_EVENTLOOP_HPP
