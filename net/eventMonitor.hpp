//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_EVENTMONITOR_HPP
#define FIRESERVER_EVENTMONITOR_HPP

#include "Channel.hpp"
#include "sys/epoll.h"
#include <vector>
#include <map>
#include <memory>


namespace Fire
{
    class eventMonitor
    {
    public:
        explicit eventMonitor();

        eventMonitor(eventMonitor &) = delete;

        eventMonitor &operator=(eventMonitor &) = delete;

        void updateChannel(Channel *);

        void removeChannel(Fire::Channel *channel);

        std::vector<Channel *> checkEvents();

    private:
        void addEvent(int fd, uint32_t event_flags);

        void modEvent(int fd, uint32_t event_flags);

        void delEvent(int fd, uint32_t event_flags);

        std::vector<Channel *> GetActivatedChannels(int count);

        const int monitor_fd;
        std::vector<epoll_event> event_details;
        std::map<int, Channel *> Fd2Channel;
    };
}

#endif //FIRESERVER_EVENTMONITOR_HPP
