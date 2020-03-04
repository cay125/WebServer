//
// Created by xiangpu on 20-2-29.
//
#include "Channel.hpp"
#include "eventLoop.hpp"
#include <sys/epoll.h>

Fire::Channel::Channel(Fire::eventLoop *loop, const int fd) : event_loop(loop), event_fd(fd)
{
    expect_event_flags |= EPOLLET;
}

void Fire::Channel::processEvent()
{
    if ((receive_event_flags & EPOLLHUP) && !(receive_event_flags & EPOLLIN))
        if (closeCallback)
            closeCallback();
    if (receive_event_flags & (EPOLLIN | EPOLLPRI | EPOLLHUP))
        if (readCallback)
            readCallback();
    if (receive_event_flags & EPOLLOUT)
        if (writeCallback)
            writeCallback();
    if (receive_event_flags & (EPOLLERR))
        if (errorCallback)
            errorCallback();
}

int Fire::Channel::GetMonitorFd()
{
    return event_fd;
}

void Fire::Channel::setREvent(uint32_t _revent)
{
    receive_event_flags = _revent;
}

void Fire::Channel::setWriteCallback(eventCallback &&cb, bool enable)
{
    Fire::Channel::writeCallback = cb;
    if (enable)
    {
        expect_event_flags |= EPOLLOUT;
        event_loop->updateChannel(this);
    }
}

void Fire::Channel::setReadCallback(eventCallback &&cb, bool enable)
{
    readCallback = cb;
    if (enable)
    {
        expect_event_flags |= EPOLLIN;
        event_loop->updateChannel(this);
    }
}

void Fire::Channel::setErrorCallback(eventCallback &&cb, bool enable)
{
    errorCallback = cb;
    if (enable)
        event_loop->updateChannel(this);
}

void Fire::Channel::setCloseCallback(Fire::Channel::eventCallback &&cb, bool enable)
{
    closeCallback = cb;
    if (enable)
        event_loop->updateChannel(this);
}

void Fire::Channel::enableWriteCallback()
{
    expect_event_flags |= EPOLLOUT;
    event_loop->updateChannel(this);
}

void Fire::Channel::disableWriteCallback()
{
    expect_event_flags &= ~EPOLLOUT;
    event_loop->updateChannel(this);
}

void Fire::Channel::enableReadCallback()
{
    expect_event_flags |= EPOLLIN;
    event_loop->updateChannel(this);
}

void Fire::Channel::enableErrorCallback()
{
    event_loop->updateChannel(this);
}

void Fire::Channel::enableCloseCallback()
{
    event_loop->updateChannel(this);
}

uint32_t Fire::Channel::getEvent()
{
    return expect_event_flags;
}

void Fire::Channel::clearCallback()
{
    readCallback = nullptr;
    writeCallback = nullptr;
    errorCallback = nullptr;
    closeCallback = nullptr;
}