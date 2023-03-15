//
// Created by zr on 23-3-13.
//

#ifndef EV_ACCEPTOR_H
#define EV_ACCEPTOR_H
#include "Socket.h"
#include "reactor/Channel.h"
#include "Types.h"
#include "utils/noncopyable.h"
#include <atomic>

namespace ev::reactor {class EventLoop;}

namespace ev::net
{
    class Acceptor : public noncopyable
    {
    public:
        Acceptor(reactor::EventLoop* loop, Inet4Address addr, bool reusePort = true);
        ~Acceptor();
        void listen();
        [[nodiscard]] bool isListening() const;
        void setNewConnectionCallback(const NewConnectionCallback& cb);

    private:
        void handleRead();

        reactor::EventLoop* loop_;
        Socket listenSocket_;
        reactor::Channel listenChannel_;
        int idleFd_;
        NewConnectionCallback newConnectionCallback_;
        std::atomic_bool listening_;
    };
}

#endif //EV_ACCEPTOR_H
