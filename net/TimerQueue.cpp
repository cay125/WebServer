//
// Created by xiangpu on 20-3-6.
//
#include <iostream>
#include <unistd.h>
#include <string.h>

#include "net/TimerQueue.hpp"
#include "net/EventLoop.hpp"

Fire::TimerQueue::TimerQueue(EventLoop *loop) : event_loop(loop), timerFd(createTimerFd()), timerChannel(event_loop, timerFd)
{
    timerChannel.setReadCallback(std::bind(&TimerQueue::HandleRead, this), true);
}

void Fire::TimerQueue::HandleRead()
{
    auto now = chrono::system_clock::now();
    readTimerFd();
    auto expired_timers = getExpiredTimer(now);
    for (auto &timer:expired_timers)
    {
        timer->Run();
        if (timer->isRepeated())
        {
            timer->UpdateExpireTime(now);
            timers.insert(std::make_pair(timer->GetExpireTime(), timer));
        }
    }
    if (!timers.empty())
    {
        auto next_expire_time = timers.begin()->first;
        resetTimerFd(next_expire_time);
    }
}

void Fire::TimerQueue::resetTimerFd(const timeStamp &next_expire_time)
{
    itimerspec new_value;
    itimerspec old_value;
    memset(&new_value, 0, sizeof(new_value));
    memset(&old_value, 0, sizeof(old_value));
    makeRestTime(new_value, next_expire_time);
    if (timerfd_settime(timerFd, 0, &new_value, &old_value) == -1)
        LOG(ERROR) << "ERROR: Reset timer fd failed";
}

std::vector<std::shared_ptr<Fire::TimerNode>> Fire::TimerQueue::getExpiredTimer(timeStamp now_timestamp)
{
    auto it_end = timers.lower_bound(now_timestamp);
    std::vector<std::shared_ptr<Fire::TimerNode>> expired;
    for (auto it = timers.begin(); it != it_end; it++)
        expired.push_back(it->second);
    timers.erase(timers.begin(), it_end);
    return expired;
}

void Fire::TimerQueue::readTimerFd()
{
    uint64_t res = 0;
    ssize_t n = read(timerFd, &res, sizeof(res));
    if (n != sizeof(res))
        LOG(ERROR) << "ERROR: Read time fd failed";
}

int Fire::TimerQueue::createTimerFd()
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0)
        LOG(ERROR) << "ERROR: Create timer fd failed";
    return fd;
}

void Fire::TimerQueue::makeRestTime(itimerspec &rest_time, const timeStamp &next_expire_time)
{
    rest_time.it_value.tv_sec = chrono::duration_cast<chrono::seconds>(next_expire_time - chrono::system_clock::now()).count();
    rest_time.it_value.tv_nsec =
            (chrono::duration_cast<chrono::microseconds>(next_expire_time - chrono::system_clock::now()).count() % 1000000) * 1000;
}

void Fire::TimerQueue::cancelTimer()
{

}

void Fire::TimerQueue::InsertTimer(std::shared_ptr<TimerNode> timer)
{
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

void Fire::TimerNode::Run()
{
    if (timerCallback)
        timerCallback();
}

Fire::timeStamp Fire::TimerNode::GetExpireTime()
{
    return expire_time;
}

std::chrono::milliseconds Fire::TimerNode::GetInterval()
{
    return interval;
}

bool Fire::TimerNode::isRepeated()
{
    return repeated;
}

void Fire::TimerNode::UpdateExpireTime(const timeStamp &now_time)
{
    if (repeated)
        expire_time = now_time + GetInterval();
}


