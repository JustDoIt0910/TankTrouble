//
// Created by zr on 23-3-15.
//

#include "utils/ThreadPool.h"
#include "utils/CountDownLatch.h"
#include <iostream>
#include <unistd.h>

int main()
{
    ev::ThreadPool pool(4, 10);
    ev::CountDownLatch latch(10);
    pool.start();
    for(int i = 0; i < 10; i++)
        pool.run([i, &latch] () {
            sleep(2.0);
            std::cout << "task " << i << "done" << std::endl;
            latch.countDown();
        });
    latch.wait();
    pool.stop();
}
