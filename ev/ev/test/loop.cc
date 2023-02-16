//
// Created by zr on 23-2-15.
//
#include "EventLoop.h"
using namespace ev;

int main()
{
    reactor::EventLoop loop;
    loop.loop();
    return 0;
}