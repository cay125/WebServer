//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_CHANNEL_HPP
#define FIRESERVER_CHANNEL_HPP

#include <functional>
#include <memory>

namespace Fire
{
    class eventLoop;

    class Channel
    {
    public:
        typedef std::function<void()> eventCallback;

        Channel(Channel &) = delete;

        Channel &operator=(Channel &) = delete;

        Channel(eventLoop *loop, int fd);

        void processEvent();

        void setREvent(uint32_t _revent);

        uint32_t getEvent();

        void clearCallback();

        void setWriteCallback(eventCallback &&cb, bool enable = true);

        void setReadCallback(eventCallback &&cb, bool enable = true);

        void setErrorCallback(eventCallback &&cb, bool enable = true);

        void setCloseCallback(eventCallback &&cb, bool enable = true);

        void enableWriteCallback();

        void disableWriteCallback();

        void enableReadCallback();

        void enableErrorCallback();

        void enableCloseCallback();

        int GetMonitorFd();

    private:
        Fire::eventLoop *event_loop;
        const int event_fd;
        uint32_t receive_event_flags, expect_event_flags;
        eventCallback writeCallback;
        eventCallback readCallback;
        eventCallback errorCallback;
        eventCallback closeCallback;
    };
}

#endif //FIRESERVER_CHANNEL_HPP
