//
// Created by zr on 23-3-5.
//

#ifndef EV_COUNT_DOWN_LATCH_H
#define EV_COUNT_DOWN_LATCH_H
#include "noncopyable.h"
#include <mutex>
#include <condition_variable>

namespace ev
{
    class CountDownLatch : public noncopyable
    {
    public:
        CountDownLatch(int count);
        ~CountDownLatch() = default;
        void wait();
        void countDown();

    private:
        std::mutex mu;
        std::condition_variable cv;
        int count_;
    };
}

#endif //EV_COUNT_DOWN_LATCH_H
