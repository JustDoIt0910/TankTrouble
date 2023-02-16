//
// Created by zr on 23-2-9.
//

#include <cassert>
#include "Channel.h"

namespace ev::reactor
{
    Channel::Channel(EventLoop* loop, int fd):
        _ownerLoop(loop),
        _fd(fd),
        interestEvents(NoneEvent),
        occurredEvents(NoneEvent),
        tied(false),
        addedToLoop(false),
        eventHandling(false),
        _status(New) {}

    Channel::~Channel()
    {
        assert(!addedToLoop);
        assert(!eventHandling);
        if(_ownerLoop->isInLoopThread())
            assert(!_ownerLoop->hasChannel(this));
    }

    void Channel::setReadCallback(ReadCallback cb) {readCallback = std::move(cb);}

    void Channel::setWriteCallback(WriteCallback cb) {writeCallback = std::move(cb);}

    void Channel::setCloseCallback(CloseCallback cb) {closeCallback = std::move(cb);}

    void Channel::setErrorCallback(ErrorCallback cb) {errorCallback = std::move(cb);}

    void Channel::enableReading()
    {
        interestEvents |= ReadEvent;
        update();
    }

    void Channel::enableWriting()
    {
        interestEvents |= WriteEvent;
        update();
    }

    void Channel::disableReading()
    {
        interestEvents &= ~ReadEvent;
        update();
    }

    void Channel::disableWriting()
    {
        interestEvents &= ~WriteEvent;
        update();
    }

    void Channel::disableAll()
    {
        interestEvents = NoneEvent;
        update();
    }

    void Channel::remove()
    {
        assert(interestEvents == NoneEvent);
        addedToLoop = false;
        _ownerLoop->removeChannel(this);
    }

    void Channel::update()
    {
        addedToLoop = true;
        _ownerLoop->updateChannel(this);
    }

    void Channel::tie(const std::shared_ptr<void>& obj)
    {
        tieWeakPtr = obj;
        tied = true;
    }

    int Channel::fd() const {return _fd;}

    Channel::ChannelStatus Channel::status() const {return _status;}

    void Channel::setStatus(ChannelStatus status) {_status = status;}

    int Channel::getInterestEvents() const {return interestEvents;}

    void Channel::setOccurredEvent(int occurred) {occurredEvents = occurred;}

    EventLoop* Channel::ownerLoop() const {return _ownerLoop;}

    void Channel::handleEvent(Timestamp receiveTime)
    {
        std::shared_ptr<void> guard;
        if(tied)
        {
            guard = tieWeakPtr.lock();
            if(guard)
                handleEventWithGuard(receiveTime);
        }
        else
            handleEventWithGuard(receiveTime);
    }

    void Channel::handleEventWithGuard(Timestamp receiveTime)
    {
        eventHandling = true;
        //读写一个已经关闭的socket, 会触发EPOLLERR错误
        if(occurredEvents & EPOLLERR)
            if(errorCallback) errorCallback();
        //读取一个peer关闭的描述符，会触发EPOLLHUP
        if((occurredEvents & EPOLLHUP) && !(occurredEvents & EPOLLIN))
            if(closeCallback) closeCallback();
        //EPOLLRDHUP在peer关闭连接或半关闭(SHUT_WR)时触发
        if(occurredEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
            if(readCallback) readCallback(receiveTime);
        if(occurredEvents & EPOLLOUT)
            if(writeCallback) writeCallback();
        eventHandling = false;
    }
}