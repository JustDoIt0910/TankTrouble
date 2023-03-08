//
// Created by zr on 23-3-5.
//

#ifndef EV_CONNECTOR_H
#define EV_CONNECTOR_H
#include <memory>
#include <functional>
#include <atomic>
#include "Inet4Address.h"
#include "Socket.h"
#include "reactor/Timer.h"
#include "utils/noncopyable.h"
#include "Types.h"

namespace ev::reactor
{
    class Channel;
    class EventLoop;
}

namespace ev::net
{
    class Socket;

    class Connector : public noncopyable,
                      public std::enable_shared_from_this<Connector>
    {
    public:
        Connector(reactor::EventLoop* loop, const Inet4Address& serverAddr);
        ~Connector();
        void setNewConnectionCallback(const NewConnectionCallback& cb);
        void start();
        void stop();
        const Inet4Address& serverAddress() const;

    private:
        enum StateE { Disconnected, Connecting, Connected };
        const static int MaxRetryDelayMs;
        const static int InitRetryDelayMs;

        void setState(StateE s);
        void startInLoop();
        void stopInLoop();
        void connect();
        void connecting();
        void handleWrite();
        void handleError();
        void retry();

        reactor::EventLoop* loop_;
        Inet4Address serverAddr_;
        std::atomic_bool started_;
        std::atomic<StateE> state_;
        std::unique_ptr<reactor::Channel> channel_;
        Socket connectingSocket_;
        NewConnectionCallback newConnectionCallback_;
        int retryDelayMs_;
        reactor::Timer::TimerId retryTimerId;
    };
}

#endif //EV_CONNECTOR_H
