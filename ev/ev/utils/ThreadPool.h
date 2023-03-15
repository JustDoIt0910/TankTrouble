//
// Created by zr on 23-3-14.
//

#ifndef EV_THREAD_POOL_H
#define EV_THREAD_POOL_H
#include <deque>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include <atomic>

namespace ev
{
    class ThreadPool
    {
    public:
        typedef std::function<void()> Task;

        ThreadPool(int threadNum, int maxQueueSize);
        void start();
        void stop();
        void run(Task task);

    private:
        void takingTask();
        std::vector<std::thread> threads_;
        std::deque<Task> tasks_;
        std::mutex mu_;
        std::condition_variable notEmpty_;
        std::condition_variable notFull_;
        std::atomic_bool running_;
        int threadNum_;
        size_t maxQueueSize_;
    };
}

#endif //EV_THREAD_POOL_H
