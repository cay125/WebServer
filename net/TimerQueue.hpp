//
// Created by xiangpu on 20-3-6.
//

#ifndef FIRESERVER_TIMERQUEUE_HPP
#define FIRESERVER_TIMERQUEUE_HPP

#include <chrono>
#include <map>
#include <iostream>
#include <stdexcept>
#include <sys/timerfd.h>
#include <glog/logging.h>

#include "net/Channel.hpp"

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
        TimerNode(Callback &&cb, const chrono::duration<rep, period> &timeout) : timerCallback(cb), expire_time(chrono::system_clock::now() + timeout)
        {}

        template<class rep1, class period1, class rep2, class period2>
        TimerNode(Callback &&cb, const chrono::duration<rep1, period1> &timeout, const chrono::duration<rep2, period2> &_interval) : timerCallback(cb), expire_time(chrono::system_clock::now() + timeout), interval(chrono::duration_cast<chrono::milliseconds>(_interval))
        {
            repeated = true;
        }

        TimerNode(Callback &&cb, tm &wakeup_time)
        {
            auto t_point = chrono::system_clock::from_time_t(mktime(&wakeup_time));
            if (t_point < chrono::system_clock::now())
            {
                std::string errstr = "Cannot create a timer which only exist in past";
                LOG(ERROR) << "ERROR: " + errstr;
                throw std::runtime_error(errstr);
            } 
            expire_time = t_point;
            timerCallback = std::move(cb);
        }

        void Run();

        timeStamp GetExpireTime();

        std::chrono::milliseconds GetInterval();

        void UpdateExpireTime(const timeStamp &now_time);

        bool isRepeated();

    private:
        bool repeated = false;
        Callback timerCallback;
        timeStamp expire_time;
        std::chrono::milliseconds interval;
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
            InsertTimer(timer);
        }

        template<class rep1, class period1, class rep2, class period2>
        void addTimer(Callback &&cb, const chrono::duration<rep1, period1> &timeout, const chrono::duration<rep2, period2> &interval)
        {
            std::shared_ptr<TimerNode> timer(new TimerNode(std::move(cb), timeout, interval));
            InsertTimer(timer);
        }

        void addTimer(Callback &&cb, tm &wakeup_time)
        {
            LOG(INFO) << "A timer task trying to create in: " << std::asctime(&wakeup_time);
            try
            {
                std::shared_ptr<TimerNode> timer(new TimerNode(std::move(cb), wakeup_time));
                LOG(INFO) << "A timer task was created in: " << std::asctime(&wakeup_time);
                InsertTimer(timer);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            } 
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

        void makeRestTime(itimerspec &rest_time, const timeStamp &next_expire_time);

        std::vector<std::shared_ptr<Fire::TimerNode>> getExpiredTimer(timeStamp now_timestamp);

        void InsertTimer(std::shared_ptr<TimerNode> timer);
    };
}

#endif //FIRESERVER_TIMERQUEUE_HPP
