//
// Created by zr on 23-3-5.
//

#include "CountDownLatch.h"

namespace ev
{
    CountDownLatch::CountDownLatch(int count):
        count_(count) {}

    void CountDownLatch::wait()
    {
        std::unique_lock<std::mutex> lk(mu);
        if(count_ > 0)
            cv.wait(lk, [this] () {return count_ == 0;});
    }

    void CountDownLatch::countDown()
    {
        std::unique_lock<std::mutex> lk(mu);
        count_--;
        if(count_ == 0)
            cv.notify_all();
    }
}