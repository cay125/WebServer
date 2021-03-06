//
// Created by xiangpu on 20-2-29.
//
#include <iostream>
#include <string.h>
#include <glog/logging.h>

#include "net/EventMonitor.hpp"

Fire::EventMonitor::EventMonitor() : monitor_fd(epoll_create1(EPOLL_CLOEXEC)), event_details(1024)
{
}

std::vector<Fire::Channel *> Fire::EventMonitor::checkEvents()
{
    /*if (Fd2Channel.empty())
    {
        std::cout << "ERROR: no event are created\n";
        return std::vector<Channel *>(0);
    }*/
    int numEvents = epoll_wait(monitor_fd, &*event_details.begin(), event_details.size(), -1);
    if (numEvents < 0)
    {
        LOG(ERROR) << "ERROR: Error occurs when waiting events happen. Reason: " << strerror(errno);
        return std::vector<Channel *>(0);
    }
    return GetActivatedChannels(numEvents);
}

std::vector<Fire::Channel *> Fire::EventMonitor::GetActivatedChannels(int count)
{
    std::vector<Channel *> res_channels;
    for (int i = 0; i < count; i++)
    {
        auto it = Fd2Channel.find(event_details[i].data.fd);
        if (it == Fd2Channel.end())
        {
            LOG(ERROR) << "ERROR: Can not find expected channel";
            return std::vector<Fire::Channel *>(0);
        }
        Channel *activated_channel = it->second;
        activated_channel->setREvent(event_details[i].events);
        res_channels.push_back(activated_channel);
        event_details[i].events = 0;
    }
    return res_channels;
}

void Fire::EventMonitor::addEvent(int fd, uint32_t event_flags)
{
    if (event_details.size() < Fd2Channel.size())
        event_details.resize(2 * event_details.size());
    epoll_event temp_event;
    bzero(&temp_event, sizeof(temp_event));
    temp_event.data.fd = fd;
    temp_event.events = event_flags;
    if (epoll_ctl(monitor_fd, EPOLL_CTL_ADD, fd, &temp_event) == -1)
    {
        LOG(ERROR) << "ERROR: Add event failed. Reason: " << strerror(errno);
    }
}

void Fire::EventMonitor::modEvent(int fd, uint32_t event_flags)
{
    epoll_event temp_event;
    bzero(&temp_event, sizeof(temp_event));
    temp_event.data.fd = fd;
    temp_event.events = event_flags;
    if (epoll_ctl(monitor_fd, EPOLL_CTL_MOD, fd, &temp_event) == -1)
    {
        LOG(ERROR) << "ERROR: Mod event failed. Reason： " << strerror(errno);
    }
}

void Fire::EventMonitor::delEvent(int fd, uint32_t event_flags)
{
    epoll_event temp_event;
    bzero(&temp_event, sizeof(temp_event));
    temp_event.data.fd = fd;
    temp_event.events = event_flags;
    if (epoll_ctl(monitor_fd, EPOLL_CTL_DEL, fd, &temp_event) == -1)
    {
        LOG(ERROR) << "ERROR: Del event failed. Reason：" << strerror(errno);
    }
}


void Fire::EventMonitor::updateChannel(Fire::Channel *channel)
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


void Fire::EventMonitor::removeChannel(Fire::Channel *channel)
{
    int fd = channel->GetMonitorFd();
    if (!Fd2Channel.count(fd))
        return;
    delEvent(fd, channel->getEvent());
    Fd2Channel.erase(fd);
}

