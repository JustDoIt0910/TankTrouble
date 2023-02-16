//
// Created by zr on 23-2-9.
//

#ifndef EV_EVENTLOOP_H
#define EV_EVENTLOOP_H
#include <functional>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <string>
#include "Timer.h"

namespace ev
{
    class Timestamp;
    class Event;
}

namespace ev::reactor
{
    class Channel;
    class EPoller;
    class TimerQueue;

    class EventLoop
    {
    public:
        typedef std::function<void()> Functor;
        typedef std::function<void(Event*)> EventListener;

        EventLoop();
        ~EventLoop();
        void loop();
        void quit();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        void wakeup() const;
        void runInLoop(Functor f);
        void queueInLoop(Functor f);
        Timer::TimerId runAt(Timestamp when, Timer::TimerTask task);
        Timer::TimerId runAfter(double delay, Timer::TimerTask task);
        Timer::TimerId runEvery(double interval, Timer::TimerTask task);
        void cancelTimer(Timer::TimerId id);
        void addEventListener(Event* event, EventListener listener);
        void dispatchEvent(Event* event);
        void assertInLoopThread() const;
        [[nodiscard]] bool isInLoopThread() const;
        [[nodiscard]] bool hasChannel(Channel* channel) const;

    private:
        void handleRead(Timestamp receiveTime) const;
        void doPendingFunctors();
        void dispatchEventInLoop(Event* event);

        const pid_t loopThread;
        int wakeupFd;
        std::atomic_bool running;
        bool eventHandling;
        bool callingPendingFunctors;
        std::unique_ptr<EPoller> poller;
        std::unique_ptr<Channel> wakeupChannel;
        std::unique_ptr<TimerQueue> timerQueue;
        std::mutex mu; //保护taskQueue
        typedef std::vector<Functor> TaskQueue;
        TaskQueue taskQueue;
        typedef std::vector<Channel*> ChannelList;
        ChannelList activeChannels;
        Channel* currentActiveChannel;
        std::unordered_map<std::string, EventListener> eventListeners;
    };
}

#endif //EV_EVENTLOOP_H
