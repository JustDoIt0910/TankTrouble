set(REACTOR_SRC EventLoop.cc
        Channel.cc
        EPoller.cc
        Timer.cc
        TimerQueue.cc
        Event.cc
        ../utils/CurrentThread.cc
        ../utils/Timestamp.cc)

add_library(reactor ${REACTOR_SRC})