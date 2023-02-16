//
// Created by zr on 23-2-14.
//

#ifndef EV_TIMER_H
#define EV_TIMER_H
#include <functional>
#include <atomic>
#include "../utils/Timestamp.h"

namespace ev::reactor
{
    class Timer
    {
    public:
        typedef std::function<void()> TimerTask;
        Timer(TimerTask task, Timestamp when, double interval);
        typedef uint64_t TimerId;
        [[nodiscard]] TimerId id() const;
        [[nodiscard]] bool repeat() const;
        void run();
        Timestamp expiration();
        void restart(Timestamp now);

    private:
        Timestamp _expiration;
        double interval;
        bool _repeat;
        TimerTask task;
        TimerId _id;

        static std::atomic_uint64_t nextId;
    };
}

#endif //EV_TIMER_H
