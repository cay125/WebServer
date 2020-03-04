//
// Created by xiangpu on 20-2-29.
//
#include "eventMonitor.hpp"
#include <iostream>

Fire::eventMonitor::eventMonitor() : monitor_fd(epoll_create1(EPOLL_CLOEXEC))
{
}

std::vector<Fire::Channel *> Fire::eventMonitor::checkEvents()
{
    if (event_details.empty())
    {
        std::cout << "ERROR: no event are created\n";
        return std::vector<Channel *>(0);
    }
    int numEvents = epoll_wait(monitor_fd, &*event_details.begin(), event_details.size(), -1);
    if (numEvents < 0)
    {
        std::cout << "ERROR: error occurs when waiting event happen\n";
        return std::vector<Channel *>(0);
    }
    return GetActivatedChannels(numEvents);
}

std::vector<Fire::Channel *> Fire::eventMonitor::GetActivatedChannels(int count)
{
    std::vector<Channel *> res_channels;
    for (int i = 0; i < count; i++)
    {
        auto it = Fd2Channel.find(event_details[i].data.fd);
        if (it == Fd2Channel.end())
        {
            std::cout << "Error: Can not find expected channel.\n";
            return std::vector<Fire::Channel *>(0);
        }
        Channel *activated_channel = it->second;
        activated_channel->setREvent(event_details[i].events);
        res_channels.push_back(activated_channel);
    }
    return res_channels;
}

void Fire::eventMonitor::addEvent(int fd, uint32_t event_flags)
{
    epoll_event temp_event;
    event_details.push_back(temp_event);
    //event_details[event_details.size() - 1].events = EPOLLIN | EPOLLET | EPOLLOUT;
    event_details[event_details.size() - 1].events = event_flags;
    event_details[event_details.size() - 1].data.fd = fd;
    if (epoll_ctl(monitor_fd, EPOLL_CTL_ADD, fd, &event_details[event_details.size() - 1]) == -1)
        std::cout << "add event failed\n";
}

void Fire::eventMonitor::modEvent(int fd, uint32_t event_flags)
{
    for (auto &event:event_details)
    {
        if (event.data.fd == fd)
        {
            event.events = event_flags;
            if (epoll_ctl(monitor_fd, EPOLL_CTL_MOD, fd, &event) == -1)
                std::cout << "mod event failed\n";
            break;
        }
    }
}

void Fire::eventMonitor::delEvent(int fd, uint32_t event_flags)
{
    for (int i = 0; i < event_details.size(); i++)
    {
        if (event_details[i].data.fd == fd)
        {
            event_details[i].events = event_flags;
            if (epoll_ctl(monitor_fd, EPOLL_CTL_DEL, fd, &event_details[i]) == -1)
                std::cout << "del event failed\n";
            event_details.erase(event_details.begin() + i);
            break;
        }
    }
}


void Fire::eventMonitor::updateChannel(Fire::Channel *channel)
{
    auto it = Fd2Channel.find(channel->GetMonitorFd());
    if (it == Fd2Channel.end())
    {
        Fd2Channel[channel->GetMonitorFd()] = channel;
        addEvent(channel->GetMonitorFd(), channel->getEvent());
    }
    else
    {
        modEvent(channel->GetMonitorFd(), channel->getEvent());
    }
}


void Fire::eventMonitor::removeChannel(Fire::Channel *channel)
{
    int fd = channel->GetMonitorFd();
    delEvent(fd, channel->getEvent());
    Fd2Channel.erase(fd);
}

