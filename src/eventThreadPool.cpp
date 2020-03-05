//
// Created by xiangpu on 20-3-4.
//
#include "eventThreadPool.hpp"
#include <iostream>

Fire::eventLoopThreadPool::eventLoopThreadPool(Fire::eventLoop *loop, int _threadNum) : baseLoop(loop), threadNum(_threadNum), threadIndex(0)
{}

void Fire::eventLoopThreadPool::Start()
{
    for (int i = 0; i < threadNum; i++)
    {
        std::shared_ptr<eventLoopThread> t(new eventLoopThread);
        threads.push_back(t);
        loops.push_back(t->start());
    }
}

Fire::eventLoop *Fire::eventLoopThreadPool::GetNextThread()
{
    if (loops.empty())
        return baseLoop;
    return loops[(threadIndex++) % threadNum];
}

Fire::eventLoopThread::eventLoopThread() : event_loop(nullptr)
{}

Fire::eventLoop *Fire::eventLoopThread::start()
{
    event_thread = std::thread(std::bind(&eventLoopThread::threadFcn, this));
    if (!event_thread.joinable())
        std::cout << "Error: thread can not detach\n";
    std::unique_lock<std::mutex> lk(m_mutex);
    while (event_loop == nullptr)
        m_cond.wait(lk, [this]()
        { return event_loop != nullptr; });
    lk.unlock();
    event_thread.detach();
    return event_loop;
}

void Fire::eventLoopThread::threadFcn()
{
    eventLoop loop;
    std::unique_lock<std::mutex> lk(m_mutex);
    event_loop = &loop;
    m_cond.notify_one();
    lk.unlock();
    loop.loop();
}
