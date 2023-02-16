//
// Created by zr on 23-2-14.
//

#include "Timer.h"

namespace ev::reactor
{
    std::atomic_uint64_t Timer::nextId = 1;

    Timer::Timer(TimerTask task, Timestamp when, double interval):
        task(std::move(task)),
        _expiration(when),
        interval(interval),
        _repeat(interval > 0.0),
        _id(nextId++) {}

    void Timer::run() {task();}

    Timestamp Timer::expiration() {return _expiration;}

    Timer::TimerId Timer::id() const {return _id;}

    bool Timer::repeat() const {return _repeat;}

    void Timer::restart(Timestamp now)
    {
        if(!_repeat)
            _expiration = Timestamp::invalid();
        else
        {
            auto ms = static_cast<int64_t>(interval * Timestamp::microSecondsPerSecond);
            _expiration.addMicroSeconds(ms);
        }
    }
}