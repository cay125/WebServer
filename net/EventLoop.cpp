//
// Created by xiangpu on 20-2-29.
//
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
#include <glog/logging.h>

#include "net/EventLoop.hpp"
#include "net/Channel.hpp"


Fire::EventLoop::EventLoop() : wakeFd(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)), wakeChannel(this, wakeFd), status(STATUS::RUNNING), own_thread_id(std::this_thread::get_id()), isDoPendng(false), timer_queue(this)
{
    LOG(INFO) << "Event loop is created in thread: " << std::this_thread::get_id();
    wakeChannel.setReadCallback(std::bind(&EventLoop::HandleRead, this), true);
}

void Fire::EventLoop::HandleRead()
{
    uint64_t res = 0;
    ssize_t n = read(wakeFd, &res, sizeof(res));
    if (n != sizeof(res))
        LOG(ERROR) << "ERROR: Read wrong bytes";
}

void Fire::EventLoop::wakeSelf()
{
    uint64_t res = 1;
    write(wakeFd, &res, sizeof(res));
}

void Fire::EventLoop::doPendingCallback()
{
    isDoPendng = true;
    std::vector<eventCallback> cbs;
    m_mutex.lock();
    cbs.swap(pendingCBs);
    m_mutex.unlock();
    for (auto &fcn:cbs)
        fcn();
    isDoPendng = false;
}

bool Fire::EventLoop::isInCurrentThread()
{
    return std::this_thread::get_id() == own_thread_id;
}

void Fire::EventLoop::runInLoop(eventCallback &&cb)
{
    if (isInCurrentThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void Fire::EventLoop::queueInLoop(Fire::EventLoop::eventCallback &&cb)
{
    m_mutex.lock();
    pendingCBs.push_back(std::move(cb));
    m_mutex.unlock();
    if (!isInCurrentThread() || isDoPendng)
        wakeSelf();
}

void Fire::EventLoop::loop()
{
    if (!isInCurrentThread())
    {
        LOG(FATAL) << "Event loop shoul only run in one thread";
    }
    status = STATUS::RUNNING;
    LOG(INFO) << "Enter event loop";
    while (status != STATUS::STOP)
    {
        std::vector<Channel *> activated_channels = monitor.checkEvents();
        if (!activated_channels.empty())
            for (auto channel :activated_channels)
                channel->processEvent();
        else
            status = STATUS::STOP;
        doPendingCallback();
    }
}

void Fire::EventLoop::stopLoop()
{
    status = STATUS::STOP;
    if (!isInCurrentThread())
        wakeSelf();
}

void Fire::EventLoop::updateChannel(Channel *channel)
{
    monitor.updateChannel(channel);
}

void Fire::EventLoop::removeChannel(Fire::Channel *channel)
{
    monitor.removeChannel(channel);
}