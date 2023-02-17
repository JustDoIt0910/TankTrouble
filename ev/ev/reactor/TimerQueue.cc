//
// Created by zr on 23-2-14.
//

#include "TimerQueue.h"
#include <sys/timerfd.h>
#include <cassert>
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include <algorithm>

namespace ev::reactor
{
    static int createTimerFd()
    {
        int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if(timerFd < 0)
        {
            // TODO handle error
        }
        return timerFd;
    }

    struct timespec howMuchTimeFromNow(Timestamp when)
    {
        Timestamp now = Timestamp::now();
        uint64_t microseconds = std::max(static_cast<int64_t>(100), when - now);
        struct timespec ts{};
        ts.tv_sec = static_cast<time_t>(
                microseconds / Timestamp::microSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
                (microseconds % Timestamp::microSecondsPerSecond) * 1000);
        return ts;
    }

    void TimerQueue::resetTimerFd(Timestamp when) const
    {
        struct itimerspec newVal{};
        bzero(&newVal,  sizeof(newVal));
        newVal.it_value = howMuchTimeFromNow(when);
        //flags为0，超时时间被解释为相对时间, 设置了TFD_TIMER_ABSTIME则为绝对时间
        int ret = ::timerfd_settime(timerFd, 0, &newVal, nullptr);
        if(ret < 0)
        {
            // TODO handle error
        }
    }

    void TimerQueue::readTimerFd() const
    {
        uint64_t timeoutCnt;
        ssize_t n = ::read(timerFd, &timeoutCnt, sizeof(timeoutCnt));
        if(n != sizeof(timeoutCnt))
        {
            // TODO handle error
        }
    }

    TimerQueue::TimerQueue(EventLoop *loop):
        ownerLoop(loop),
        timerFd(createTimerFd()),
        timerChannel(new Channel(loop, timerFd)),
        callingExpiredTimers(false)
    {
        timerChannel->setReadCallback(std::bind(&TimerQueue::handleRead, this));
        timerChannel->enableReading();
    }

    TimerQueue::~TimerQueue()
    {
        timerChannel->disableAll();
        timerChannel->remove();
    }

    Timer::TimerId TimerQueue::addTimer(Timestamp when, double interval, Timer::TimerTask task)
    {
        auto timer = new Timer(std::move(task), when, interval);
        ownerLoop->runInLoop([this, timer] { addTimerInLoop(timer); });
        return timer->id();
    }

    void TimerQueue::addTimerInLoop(Timer* timer)
    {
        ownerLoop->assertInLoopThread();
        bool earliestChanged = insert(timer);
        if(earliestChanged)
            resetTimerFd(timer->expiration());
    }

    void TimerQueue::cancelTimer(Timer::TimerId id)
    {
        ownerLoop->runInLoop([this, id]{ cancelTimerInLoop(id);});
    }

    void TimerQueue::cancelTimerInLoop(Timer::TimerId id)
    {
        ownerLoop->assertInLoopThread();
        if(callingExpiredTimers)
            cancelingTimers.insert(id);
        else if(ids.find(id) != ids.end())
        {
            assert(keys.find(ids[id]) != keys.end());
            keys.erase(ids[id]);
            timers.erase(id);
            ids.erase(id);
        }
    }

    bool TimerQueue::insert(Timer *timer)
    {
        Key key = std::make_pair(timer->expiration(), timer->id());
        auto timerListEntry = std::make_pair(timer->id(), std::unique_ptr<Timer>(timer));
        bool earliestChanged = false;
        auto earliest = keys.cbegin();
        /* 之前timers里没有timer或者新timer超时时间早于timers里第一个timer
         * 说明timerFd的超时时间需要调整 */
        if(earliest == keys.cend() || key.first < earliest->first)
            earliestChanged = true;
        {
            auto result = timers.insert(std::move(timerListEntry));
            assert(result.second);
        }
        {
            auto result = keys.insert(key);
            assert(result.second);
        }
        {
            auto timerIdEntry = std::make_pair(timer->id(), key);
            auto result = ids.insert(timerIdEntry);
            assert(result.second);
        }
        return earliestChanged;
    }

    void TimerQueue::handleRead()
    {
        ownerLoop->assertInLoopThread();
        Timestamp now = Timestamp::now();
        readTimerFd();
        getExpired(now);
        cancelingTimers.clear();
        callingExpiredTimers = true;
        for(const Key& key: expiredTimers)
        {
            assert(timers.find(key.second) != timers.end());
            timers[key.second]->run();
        }
        callingExpiredTimers = false;
        reset(now);
    }

    void TimerQueue::getExpired(Timestamp now)
    {
        expiredTimers.clear();
        Key sentry = std::make_pair(now, UINT64_MAX);
        auto firstGreater = keys.lower_bound(sentry);
        std::copy(keys.begin(), firstGreater, std::back_inserter(expiredTimers));
        keys.erase(keys.begin(), firstGreater);
    }

    void TimerQueue::reset(Timestamp now)
    {
        for(const Key& key: expiredTimers)
        {
            auto& timer = timers[key.second];
            if(timer->repeat() && cancelingTimers.find(key.second) == cancelingTimers.end())
            {
                timer->restart(now);
                auto newKey = std::make_pair(timer->expiration(), timer->id());
                auto result = keys.insert(newKey);
                assert(result.second);
                ids[timer->id()] = newKey;
            }
            else
            {
                ids.erase(key.second);
                timers.erase(key.second);
            }
        }
        if(!keys.empty())
        {
            Timestamp next = keys.begin()->first;
            if(next.isValid())
                resetTimerFd(next);
        }
    }
}