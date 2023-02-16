//
// Created by zr on 23-2-14.
//

#ifndef EV_TIMERQUEUE_H
#define EV_TIMERQUEUE_H
#include "Channel.h"
#include "Timer.h"
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

namespace ev::reactor
{
    class EventLoop;

    class TimerQueue
    {
    public:
        explicit TimerQueue(EventLoop* loop);
        ~TimerQueue();
        Timer::TimerId addTimer(Timestamp when, double interval, Timer::TimerTask task);
        void cancelTimer(Timer::TimerId);

    private:
        void handleRead();
        void addTimerInLoop(Timer* timer);
        void cancelTimerInLoop(Timer::TimerId id);
        bool insert(Timer* timer);
        void getExpired(Timestamp now);
        void reset(Timestamp now);
        void resetTimerFd(Timestamp when) const;
        void readTimerFd() const;

        int timerFd;
        std::unique_ptr<Channel> timerChannel;
        EventLoop* ownerLoop;

        typedef std::pair<Timestamp, Timer::TimerId> Key;
        typedef std::set<Key> KeyList;
        typedef std::unordered_map<Timer::TimerId, std::unique_ptr<Timer>> TimerList;
        typedef std::unordered_map<Timer::TimerId, Key> TimerIdList;
        KeyList keys;
        TimerList timers;
        TimerIdList ids;

        std::vector<Key> expiredTimers;
        std::unordered_set<Timer::TimerId> cancelingTimers;
        bool callingExpiredTimers;
    };
}

#endif //EV_TIMERQUEUE_H
