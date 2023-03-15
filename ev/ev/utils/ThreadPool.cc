//
// Created by zr on 23-3-14.
//

#include "ThreadPool.h"
#include <iostream>

namespace ev
{
    ThreadPool::ThreadPool(int threadNum, int maxQueueSize):
        threadNum_(threadNum),
        maxQueueSize_(maxQueueSize),
        running_(false) {threads_.resize(threadNum_);}

    void ThreadPool::start()
    {
        if(!running_.exchange(true))
        {
            for(int i = 0; i < threadNum_; i++)
                threads_[i] = std::thread([this] () {takingTask();});
        }
    }

    void ThreadPool::stop()
    {
        if(running_.exchange(false))
        {
            std::unique_lock<std::mutex> lk(mu_);
            notEmpty_.notify_all();
            notFull_.notify_all();
        }
        for(std::thread& t: threads_)
            t.join();
    }

    void ThreadPool::run(Task task)
    {
        std::unique_lock<std::mutex> lk(mu_);
        while(tasks_.size() >= maxQueueSize_ && running_.load())
            notFull_.wait(lk);
        if(!running_.load())
            return;
        tasks_.push_back(std::move(task));
        notEmpty_.notify_one();
    }

    void ThreadPool::takingTask()
    {
        try
        {
            while(running_.load())
            {
                Task task;
                {
                    std::unique_lock<std::mutex> lk(mu_);
                    while(tasks_.empty() && running_.load())
                        notEmpty_.wait(lk);
                    if(!running_.load())
                        break;
                    task = tasks_.front();
                    tasks_.pop_front();
                    notFull_.notify_one();
                }
                if(task) task();
            }
        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "Exception caught in ThreadPool: %s", e.what());
        }
    }
}