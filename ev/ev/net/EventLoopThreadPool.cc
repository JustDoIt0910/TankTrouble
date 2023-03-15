//
// Created by zr on 23-3-14.
//

#include "EventLoopThreadPool.h"
#include "reactor/EventLoop.h"
#include "EventLoopThread.h"
#include <cassert>

using namespace ev::reactor;

namespace ev::net
{
    EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
            : baseLoop_(baseLoop),
              started_(false),
              numThreads_(0),
              next_(0) {}

    EventLoopThreadPool::~EventLoopThreadPool() = default;

    bool EventLoopThreadPool::started() const { return started_; }

    void EventLoopThreadPool::start()
    {
        assert(!started_);
        baseLoop_->assertInLoopThread();
        started_ = true;
        for (int i = 0; i < numThreads_; ++i)
        {
            auto t = std::make_unique<EventLoopThread>();
            threads_.push_back(std::move(t));
            loops_.push_back(threads_[i]->startLoop());
        }
    }

    EventLoop* EventLoopThreadPool::getNextLoop()
    {
        baseLoop_->assertInLoopThread();
        assert(started_);
        EventLoop* loop = baseLoop_;
        if (!loops_.empty())
        {
            loop = loops_[next_++];
            if(next_ >= loops_.size())
                next_ = 0;
        }
        return loop;
    }

    std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
    {
        baseLoop_->assertInLoopThread();
        assert(started_);
        if (loops_.empty())
            return {baseLoop_};
        else
            return loops_;
    }
}