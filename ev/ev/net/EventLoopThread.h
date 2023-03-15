//
// Created by zr on 23-3-14.
//

#ifndef EV_EVENT_LOOP_THREAD_H
#define EV_EVENT_LOOP_THREAD_H
#include "utils/noncopyable.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ev::reactor{class EventLoop;}

namespace ev::net
{
    class EventLoopThread : public noncopyable
    {
    public:

        EventLoopThread();
        ~EventLoopThread();
        reactor::EventLoop* startLoop();

    private:
        void threadFunc();

        reactor::EventLoop* loop_;
        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };
}

#endif //EV_EVENT_LOOP_THREAD_H
