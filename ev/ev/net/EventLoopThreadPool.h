//
// Created by zr on 23-3-14.
//

#ifndef EV_EVENT_LOOP_THREAD_POOL_H
#define EV_EVENT_LOOP_THREAD_POOL_H
#include "utils/noncopyable.h"
#include <vector>
#include <memory>

namespace ev::reactor {class EventLoop;}

namespace ev::net
{
    class EventLoopThread;
    class EventLoopThreadPool : public noncopyable
    {
    public:
        explicit EventLoopThreadPool(reactor::EventLoop* baseLoop);
        ~EventLoopThreadPool();
        void setThreadNum(int numThreads) { numThreads_ = numThreads; }
        void start();

        reactor::EventLoop* getNextLoop();
        std::vector<reactor::EventLoop*> getAllLoops();
        [[nodiscard]] bool started() const;

    private:
        reactor::EventLoop* baseLoop_;
        bool started_;
        int numThreads_;
        size_t next_;
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<reactor::EventLoop*> loops_;
    };
}

#endif //EV_EVENT_LOOP_THREAD_POOL_H
