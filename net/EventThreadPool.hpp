//
// Created by xiangpu on 20-3-4.
//

#ifndef FIRESERVER_EVENTTHREADPOOL_HPP
#define FIRESERVER_EVENTTHREADPOOL_HPP

#include <thread>
#include <mutex>
#include <condition_variable>

#include "net/EventLoop.hpp"

namespace Fire
{
    class EventLoopThread
    {
    public:
        explicit EventLoopThread();

        EventLoop *start();

    private:
        EventLoop *event_loop;
        std::thread event_thread;
        std::mutex m_mutex;
        std::condition_variable m_cond;

        void threadFcn();
    };

    class EventLoopThreadPool
    {
    public:
        explicit EventLoopThreadPool(EventLoop *loop, int _threadNum);

        void Start();

        EventLoop *GetNextThread();

    private:
        uint32_t threadNum, threadIndex;
        EventLoop *baseLoop;
        std::vector<Fire::EventLoop *> loops;
        std::vector<std::shared_ptr<Fire::EventLoopThread>> threads;
    };
}

#endif //FIRESERVER_EVENTTHREADPOOL_HPP
