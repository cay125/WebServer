//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_EVENTLOOP_HPP
#define FIRESERVER_EVENTLOOP_HPP

#include <thread>
#include <mutex>

#include "net/Channel.hpp"
#include "net/EventMonitor.hpp"

namespace Fire
{
    class EventLoop
    {
        typedef std::function<void()> eventCallback;
    public:
        enum class STATUS
        {
            STOP, RUNNING, SLEEP
        };

        explicit EventLoop();

        ~EventLoop() = default;

        EventLoop(EventLoop &) = delete;

        EventLoop &operator=(EventLoop &) = delete;

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
        EventMonitor monitor;
        std::mutex m_mutex;
        int wakeFd;
        Channel wakeChannel;
        std::vector<Fire::EventLoop::eventCallback> pendingCBs;
        bool isDoPendng;
    };
}

#endif //FIRESERVER_EVENTLOOP_HPP
