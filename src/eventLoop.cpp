//
// Created by xiangpu on 20-2-29.
//
#include "eventLoop.hpp"
#include "Channel.hpp"
#include <iostream>

Fire::eventLoop::eventLoop() : isLooping(false), own_thread_id(std::this_thread::get_id())
{
    std::cout << "Event loop is created in thread: " << std::this_thread::get_id() << "\n";
}

void Fire::eventLoop::loop()
{
    if (std::this_thread::get_id() != own_thread_id)
    {
        std::cout << "Event loop shoul only run in one thread\n";
        exit(-1);
    }
    isLooping = true;
    std::cout << "enter event loop\n";
    while (isLooping)
    {
        std::vector<Channel *> activated_channels = monitor.checkEvents();
        if (!activated_channels.empty())
            for (auto channel :activated_channels)
                channel->processEvent();
//        else
//            isLooping = false;
    }
}

void Fire::eventLoop::stopLoop()
{
    isLooping = false;
}

void Fire::eventLoop::updateChannel(Channel *channel)
{
    monitor.updateChannel(channel);
}

void Fire::eventLoop::removeChannel(Fire::Channel *channel)
{
    monitor.removeChannel(channel);
}