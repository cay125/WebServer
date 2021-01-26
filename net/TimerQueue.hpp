//
// Created by xiangpu on 20-3-6.
//

#ifndef FIRESERVER_TIMERQUEUE_HPP
#define FIRESERVER_TIMERQUEUE_HPP

#include <chrono>
#include <sys/timerfd.h>

#include "net/Channel.hpp"
#include "net/EventLoop.hpp"

namespace Fire
{
    namespace chrono=std::chrono;
    typedef chrono::time_point<chrono::system_clock, chrono::nanoseconds> timeStamp;

    class TimerNode
    {
        typedef std::function<void()> Callback;
    public:
        TimerNode(TimerNode &) = delete;

        TimerNode &operator=(TimerNode &) = delete;

        template<class rep, class period>
        TimerNode(Callback &&cb, const chrono::duration<rep, period> &timeout):timerCallback(cb),
                                                                               expire_time(chrono::system_clock::now() + timeout)
        {

        }

        void Run();

        timeStamp GetExpireTime();

        //private:
        Callback timerCallback;
        timeStamp expire_time;
    };

    class TimerQueue
    {
        typedef std::function<void()> Callback;
    public:
        explicit TimerQueue(EventLoop *loop);

        template<class rep, class period>
        void addTimer(Callback &&cb, const chrono::duration<rep, period> &timeout)
        {
            std::shared_ptr<TimerNode> timer(new TimerNode(std::move(cb), timeout));
            event_loop->runInLoop([this, timer]()
                                  {
                                      bool isChanged = false;
                                      if (timers.empty() || timer->GetExpireTime() < timers.begin()->first)
                                          isChanged = true;
                                      timers.insert(std::make_pair(timer->GetExpireTime(), timer));
                                      if (isChanged)
                                          resetTimerFd(timer->GetExpireTime());
                                  });
        }

        void cancelTimer();

    private:
        EventLoop *event_loop;
        int timerFd;
        Channel timerChannel;
        std::multimap<timeStamp, std::shared_ptr<Fire::TimerNode>> timers;

        static int createTimerFd();

        void readTimerFd();

        void resetTimerFd(const timeStamp &next_expire_time);

        void HandleRead();

        void makeRestTime(itimerspec &rest_time, timeStamp next_expire_time);

        std::vector<std::shared_ptr<Fire::TimerNode>> getExpiredTimer();
    };
}

#endif //FIRESERVER_TIMERQUEUE_HPP