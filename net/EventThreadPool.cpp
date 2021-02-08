//
// Created by xiangpu on 20-3-4.
//
#include <iostream>
#include <glog/logging.h>

#include "net/EventThreadPool.hpp"

Fire::EventLoopThreadPool::EventLoopThreadPool(Fire::EventLoop *loop, int _threadNum) : baseLoop(loop), threadNum(_threadNum), threadIndex(0)
{}

void Fire::EventLoopThreadPool::Start()
{
    for (int i = 0; i < threadNum; i++)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread);
        threads.push_back(t);
        loops.push_back(t->start());
    }
}

Fire::EventLoop *Fire::EventLoopThreadPool::GetNextThread()
{
    if (loops.empty())
        return baseLoop;
    return loops[(threadIndex++) % threadNum];
}

Fire::EventLoopThread::EventLoopThread() : event_loop(nullptr)
{}

Fire::EventLoop *Fire::EventLoopThread::start()
{
    event_thread = std::thread(std::bind(&EventLoopThread::threadFcn, this));
    if (!event_thread.joinable())
        LOG(ERROR) << "ERROR: Thread can not detach";
    std::unique_lock<std::mutex> lk(m_mutex);
    while (event_loop == nullptr)
        m_cond.wait(lk, [this]()
        { return event_loop != nullptr; });
    lk.unlock();
    event_thread.detach();
    return event_loop;
}

void Fire::EventLoopThread::threadFcn()
{
    EventLoop loop;
    std::unique_lock<std::mutex> lk(m_mutex);
    event_loop = &loop;
    m_cond.notify_one();
    lk.unlock();
    loop.loop();
}
