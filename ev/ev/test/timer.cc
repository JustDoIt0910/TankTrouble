//
// Created by zr on 23-2-15.
//
#include "reactor/EventLoop.h"
#include <iostream>
using namespace ev;

int main()
{
    reactor::EventLoop loop;
    loop.runEvery(1.0, []{
        std::cout << "timeout1: " << Timestamp::now().toFormattedString() << std::endl;
    });
//    loop.runEvery(2.0, []{
//        std::cout << "timeout2: " << Timestamp::now().toFormattedString() << std::endl;
//    });
//    loop.runEvery(3.0, []{
//        std::cout << "timeout3: " << Timestamp::now().toFormattedString() << std::endl;
//    });
//    loop.runAfter(60.0, []{
//        std::cout << "timeout4: " << Timestamp::now().toFormattedString() << std::endl;
//    });
    loop.runEvery(0.02, []{
        std::cout << "timeout5: " << Timestamp::now().toFormattedString() << std::endl;
    });

    loop.loop();
    return 0;
}