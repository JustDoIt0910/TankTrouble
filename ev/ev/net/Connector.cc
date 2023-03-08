//
// Created by zr on 23-3-5.
//

#include "Connector.h"
#include <memory>
#include "reactor/Channel.h"
#include "reactor/EventLoop.h"

namespace ev::net
{
    const int Connector::MaxRetryDelayMs = 30 * 1000;
    const int Connector::InitRetryDelayMs = 500;

    Connector::Connector(reactor::EventLoop* loop, const Inet4Address& serverAddr):
        loop_(loop),
        serverAddr_(serverAddr),
        started_(false),
        state_(Disconnected),
        channel_(nullptr),
        retryDelayMs_(InitRetryDelayMs),
        retryTimerId(0) {}

    Connector::~Connector() = default;

    void Connector::start()
    {
        started_ = true;
        loop_->runInLoop([this] () {startInLoop();});
    }

    void Connector::stop()
    {
        started_ = false;
        loop_->runInLoop([this] () {stopInLoop();});
    }

    void Connector::stopInLoop()
    {
        loop_->assertInLoopThread();
        if(state_.load() == Connecting)
        {
            channel_->disableAll();
            channel_->remove();
            channel_.reset();
            connectingSocket_.close();
            loop_->cancelTimer(retryTimerId);
        }
    }

    void Connector::startInLoop()
    {
        loop_->assertInLoopThread();
        if(started_ && state_.load() == Disconnected)
            connect();
    }

    void Connector::connect()
    {
        connectingSocket_ = Socket(serverAddr_.family());
        int ret = connectingSocket_.connect(serverAddr_);
        int savedErrno = (ret == 0) ? 0 : errno;
        switch (savedErrno)
        {
            case 0:
            case EINPROGRESS:
            case EISCONN:
                connecting();
                break;
            case EAGAIN:
            case EADDRINUSE:
            case EADDRNOTAVAIL:
            case ECONNREFUSED:
            case ENETUNREACH:
                retry();
                break;
            default:
                connectingSocket_.close();
                break;
        }
    }

    void Connector::connecting()
    {
        setState(Connecting);
        channel_ = std::make_unique<reactor::Channel>(loop_, connectingSocket_.fd());
        channel_->setWriteCallback([guard = shared_from_this()] () {
            guard->handleWrite();
        });
        channel_->setErrorCallback([guard = shared_from_this()] () {
            guard->handleError();
        });
        channel_->enableWriting();
    }

    void Connector::handleWrite()
    {
        if(state_.load() == Connecting)
        {
            channel_->disableAll();
            channel_->remove();
            int err = connectingSocket_.error();
            if(err == 0)
            {
                setState(Connected);
                if(started_ && newConnectionCallback_)
                    newConnectionCallback_(std::move(connectingSocket_));
                else
                    connectingSocket_.close();
            }
            else
                retry();
        }
    }

    void Connector::handleError()
    {
        if(state_.load() == Connecting)
        {
            channel_->disableAll();
            channel_->remove();
            retry();
        }
    }

    void Connector::retry()
    {
        setState(Disconnected);
        if (started_)
        {
            retryTimerId = loop_->runAfter(retryDelayMs_ / 1000.0,
                            [guard = shared_from_this()] () {
                guard->startInLoop();
            });
            retryDelayMs_ = std::min(retryDelayMs_ * 2, MaxRetryDelayMs);
        }
    }

    void Connector::setState(StateE s) { state_.store(s); }

    void Connector::setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

    const Inet4Address& Connector::serverAddress() const { return serverAddr_; }
}