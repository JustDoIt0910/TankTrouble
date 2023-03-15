//
// Created by zr on 23-3-14.
//
#include "EventLoopThread.h"
#include "reactor/EventLoop.h"

using namespace ev::reactor;

namespace ev::net
{
    EventLoopThread::EventLoopThread()
            : loop_(nullptr) {}

    EventLoopThread::~EventLoopThread()
    {
        if (loop_ != NULL)
        {
            loop_->quit();
            thread_.join();
        }
    }

    EventLoop* EventLoopThread::startLoop()
    {
        thread_ = std::thread([this] () {threadFunc();});
        reactor::EventLoop* loop = nullptr;
        {
            std::unique_lock<std::mutex> lk(mutex_);
            cond_.wait(lk, [this] () {return loop_ != nullptr;});
            loop = loop_;
        }
        return loop;
    }

    void EventLoopThread::threadFunc()
    {
        EventLoop loop;
        {
            std::unique_lock<std::mutex> lk(mutex_);
            loop_ = &loop;
            cond_.notify_all();
        }
        loop.loop();
        std::unique_lock<std::mutex> lk(mutex_);
        loop_ = nullptr;
    }
}
