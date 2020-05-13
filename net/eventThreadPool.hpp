//
// Created by xiangpu on 20-3-4.
//

#ifndef FIRESERVER_EVENTTHREADPOOL_HPP
#define FIRESERVER_EVENTTHREADPOOL_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include "eventLoop.hpp"

namespace Fire
{
    class eventLoopThread
    {
    public:
        explicit eventLoopThread();

        eventLoop *start();

    private:
        eventLoop *event_loop;
        std::thread event_thread;
        std::mutex m_mutex;
        std::condition_variable m_cond;

        void threadFcn();
    };

    class eventLoopThreadPool
    {
    public:
        explicit eventLoopThreadPool(eventLoop *loop, int _threadNum);

        void Start();

        eventLoop *GetNextThread();

    private:
        uint32_t threadNum, threadIndex;
        eventLoop *baseLoop;
        std::vector<Fire::eventLoop *> loops;
        std::vector<std::shared_ptr<Fire::eventLoopThread>> threads;
    };
}

#endif //FIRESERVER_EVENTTHREADPOOL_HPP
