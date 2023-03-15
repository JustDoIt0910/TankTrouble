//
// Created by zr on 23-3-13.
//

#include "Acceptor.h"
#include "reactor/EventLoop.h"
#include <fcntl.h>
#include <unistd.h>

namespace ev::net
{
    Acceptor::Acceptor(reactor::EventLoop* loop, Inet4Address addr, bool reusePort):
        loop_(loop),
        listenSocket_(addr.family()),
        listenChannel_(loop_, listenSocket_.fd()),
        idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
        listening_(false)
    {
        listenSocket_.setReusePort(true);
        if(reusePort)
            listenSocket_.setReusePort(true);
        listenChannel_.setReadCallback([this] (Timestamp) {handleRead();});
        listenSocket_.bind(addr);
    }

    Acceptor::~Acceptor()
    {
        listenChannel_.disableAll();
        listenChannel_.remove();
    }

    void Acceptor::listen()
    {
        if(!listening_.exchange(true))
        {
            loop_->assertInLoopThread();
            listenSocket_.listen();
            listenChannel_.enableReading();
        }
    }

    bool Acceptor::isListening() const {return listening_.load();}

    void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb) {newConnectionCallback_ = cb;}

    void Acceptor::handleRead()
    {
        loop_->assertInLoopThread();
        Inet4Address peerAddr{};
        Socket conn = listenSocket_.accept(peerAddr);
        if (conn.isValid())
        {
            if (newConnectionCallback_)
                newConnectionCallback_(std::move(conn));
        }
        else
        {
            if (errno == EMFILE)
            {
                ::close(idleFd_);
                idleFd_ = ::accept(listenSocket_.fd(), nullptr, nullptr);
                ::close(idleFd_);
                idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            }
        }
    }
}