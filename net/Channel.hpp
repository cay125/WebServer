//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_CHANNEL_HPP
#define FIRESERVER_CHANNEL_HPP

#include <functional>
#include <memory>

namespace Fire
{
    class EventLoop;

    class Channel
    {
    public:
        enum RegisterStatusMask
        {
            write = 0x01, read = 0x02
        };

        enum class TriggerMode : uint8_t {edge_mode = 0x01, level_mode = 0x02};

        typedef std::function<void()> eventCallback;

        Channel(Channel &) = delete;

        Channel &operator=(Channel &) = delete;

        Channel(EventLoop *loop, int fd, TriggerMode mode = TriggerMode::edge_mode);

        void processEvent();

        void setREvent(uint32_t _revent);

        uint32_t getEvent();

        void clearCallback();

        void remove();

        void setWriteCallback(eventCallback &&cb, bool enable = true);

        void setReadCallback(eventCallback &&cb, bool enable = true);

        // note: parameter 'enable' is ignored, for epoll will always report err events
        void setErrorCallback(eventCallback &&cb, bool enable = true);

        // note: parameter 'enable' is ignored, for epoll will always report hang up events
        void setCloseCallback(eventCallback &&cb, bool enable = true);

        void enableWriteCallback();

        void disableWriteCallback();

        void enableReadCallback();

        void disableReadCallback();

        void enableErrorCallback();

        void enableCloseCallback();

        void disableAll();// disable read and write

        uint8_t GetRegisterStatus();

        TriggerMode GetTriggerMode();

        int GetMonitorFd();

    private:
        Fire::EventLoop *event_loop;
        uint8_t register_status;
        const int event_fd;
        const TriggerMode trigger_mode;
        uint32_t receive_event_flags, expect_event_flags;
        eventCallback writeCallback;
        eventCallback readCallback;
        eventCallback errorCallback;
        eventCallback closeCallback;
    };
}

#endif //FIRESERVER_CHANNEL_HPP
