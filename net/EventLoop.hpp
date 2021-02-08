//
// Created by xiangpu on 20-2-29.
//

#ifndef FIRESERVER_EVENTLOOP_HPP
#define FIRESERVER_EVENTLOOP_HPP

#include <thread>
#include <mutex>
#include <sstream>
#include <iomanip>

#include "net/Channel.hpp"
#include "net/EventMonitor.hpp"
#include "net/TimerQueue.hpp"

namespace Fire
{
    class EventLoop
    {
        typedef std::function<void()> eventCallback;
    public:
        enum class STATUS
        {
            STOP, RUNNING, SLEEP
        };

        explicit EventLoop();

        ~EventLoop() = default;

        EventLoop(EventLoop &) = delete;

        EventLoop &operator=(EventLoop &) = delete;

        void loop();

        void updateChannel(Channel *channel);

        void removeChannel(Channel *channel);

        void runInLoop(eventCallback &&cb);

        void queueInLoop(eventCallback &&cb);

        void stopLoop();

        template<class rep1, class period1, class rep2, class period2>
        void RunEvery(eventCallback &&cb, const chrono::duration<rep1, period1> &timeout, const chrono::duration<rep2, period2> &interval)
        {
            timer_queue.addTimer(std::move(cb), timeout, interval);
        }

        void RunAt(eventCallback &&cb, std::string wakeup_time, std::string format)
        {
            tm t{};
            std::istringstream ss(wakeup_time);
            ss >> std::get_time(&t, format.c_str());
            if (ss.fail())
            {
                LOG(ERROR) << "ERROR: Parsing time point string failed";
                return;
            }
            timer_queue.addTimer(std::move(cb), t);
        }

        template<class rep, class period>
        void RunAfter(eventCallback &&cb, const chrono::duration<rep, period> &timeout)
        {
            timer_queue.addTimer(std::move(cb), timeout);
        }

        STATUS getStatus()
        { return status; }


    private:
        void doPendingCallback();

        bool isInCurrentThread();

        void HandleRead();

        void wakeSelf();

        std::thread::id own_thread_id;
        STATUS status;
        EventMonitor monitor;
        std::mutex m_mutex;
        int wakeFd;
        Channel wakeChannel;
        std::vector<Fire::EventLoop::eventCallback> pendingCBs;
        bool isDoPendng;
        TimerQueue timer_queue;
    };
}

#endif //FIRESERVER_EVENTLOOP_HPP
