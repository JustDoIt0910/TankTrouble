//
// Created by zr on 23-3-4.
//

#include "TcpConnection.h"

#include <utility>
#include "reactor/EventLoop.h"
#include "reactor/Channel.h"

namespace ev::net
{
    const size_t TcpConnection::DefaultHighWaterMark = 64 * 1024 * 1024; // 64MB 高水位标志

    TcpConnection::TcpConnection(reactor::EventLoop *loop, Socket &&socket,
                                 const ev::net::Inet4Address &localAddr,
                                 const ev::net::Inet4Address &peerAddr):
        loop_(loop),
        state_(Connecting),
        socket_(std::move(socket)),
        channel_(new reactor::Channel(loop, socket_.fd())),
        localAddr_(localAddr),
        peerAddr_(peerAddr),
        highWaterMark_(DefaultHighWaterMark)
    {
        channel_->setReadCallback([this] (Timestamp receiveTime) {this->handleRead(receiveTime);});
        channel_->setWriteCallback([this] () {this->handleWrite();});
        channel_->setCloseCallback([this] () {this->handleClose();});
        channel_->setErrorCallback([this] () {this->handleError();});
        socket_.setKeepAlive(true);
    }

    TcpConnection::~TcpConnection() { assert(state_.load() == Disconnected);}

    void TcpConnection::connectEstablished()
    {
        loop_->assertInLoopThread();
        assert(state_.load() == Connecting);
        setState(Connected);
        channel_->tie(shared_from_this());
        if(connectionCallback_)
            connectionCallback_(shared_from_this());
        channel_->enableReading();
    }

    void TcpConnection::startRead() {loop_->runInLoop([this] () {this->startReadInLoop();});}

    void TcpConnection::stopRead() {loop_->runInLoop([this] () {this->stopReadInLoop();});}

    void TcpConnection::startReadInLoop()
    {
        loop_->assertInLoopThread();
        if(!channel_->isReading())
            channel_->enableReading();
    }

    void TcpConnection::stopReadInLoop()
    {
        loop_->assertInLoopThread();
        if(channel_->isReading())
            channel_->disableReading();
    }

    void TcpConnection::shutdown()
    {
        if(state_.exchange(Disconnecting) == Connected)
        {
            loop_->runInLoop([guard = shared_from_this()] {
                guard->shutdownInLoop();
            });
        }
    }

    void TcpConnection::shutdownInLoop()
    {
        loop_->assertInLoopThread();
        assert(state_.load() == Disconnecting);
        if(!channel_->isWriting()) //channel_还在关注写事件，说明缓冲区数据还没发送完，不能关闭
            socket_.shutdownWrite();
    }

    void TcpConnection::forceClose()
    {
        if (state_.exchange(Disconnecting) != Disconnected)
            loop_->queueInLoop([guard = shared_from_this()] () {
                guard->forceCloseInLoop();
            });
    }

    void TcpConnection::forceCloseInLoop()
    {
        loop_->assertInLoopThread();
        if (state_.load() != Disconnected)
            handleClose();
    }

    void TcpConnection::send(const void* data, int len)
    {
        Buffer buf;
        buf.append(data, len);
        send(buf);
    }

    void TcpConnection::send(Buffer& buf)
    {
        if (state_.load() == Connected)
        {
            if (loop_->isInLoopThread())
            {
                sendInLoop(buf.peek(), buf.readableBytes());
                buf.retrieveAll();
            }
            else
            {
                loop_->queueInLoop([guard = shared_from_this(),
                                    data = buf.retrieveAllAsString()] () {
                    guard->sendInLoop(data);
                });
            }
        }
    }

    void TcpConnection::sendInLoop(const std::string& str) { sendInLoop(str.data(), str.size());}

    void TcpConnection::sendInLoop(const void* data, size_t len)
    {
        loop_->assertInLoopThread();
        if (state_.load() != Connected)
            return;
        ssize_t nwrote = 0;
        size_t remaining = len;
        bool faultError = false;
        if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
        {
            nwrote = socket_.write(data, len);
            if (nwrote >= 0)
            {
                remaining = len - nwrote;
                if (remaining == 0 && writeCompleteCallback_)
                    loop_->queueInLoop([guard = shared_from_this()] () {
                        guard->writeCompleteCallback_(guard);
                    });
            }
            else
            {
                nwrote = 0;
                // EAGAIN 或者 EWOULDBLOCK 说明发送缓冲满了，应该重试
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    //对方已经关闭连接
                    if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                    {
                        faultError = true;
                    }
                }
            }
        }
        assert(remaining <= len);
        if (!faultError && remaining > 0)
        {
            size_t oldLen = outputBuffer_.readableBytes();
            if (oldLen + remaining >= highWaterMark_
                && oldLen < highWaterMark_
                && highWaterMarkCallback_)
            {
                loop_->queueInLoop([guard = shared_from_this(), oldLen, remaining] () {
                    guard->highWaterMarkCallback_(guard, oldLen + remaining);
                });
            }
            outputBuffer_.append(static_cast<const char*>(data) +nwrote, remaining);
            if (!channel_->isWriting())
                channel_->enableWriting();
        }
    }

    void TcpConnection::handleRead(Timestamp receiveTime)
    {
        loop_->assertInLoopThread();
        int savedErrno = 0;
        ssize_t n = inputBuffer_.readFd(socket_.fd(), &savedErrno);
        if (n > 0)
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        // read返回0, 执行关闭的唯一条件
        else if (n == 0)
            handleClose();
        else
        {
            errno = savedErrno;
            handleError();
        }
    }

    void TcpConnection::handleWrite()
    {
        loop_->assertInLoopThread();
        if (channel_->isWriting())
        {
            ssize_t n = socket_.write(outputBuffer_.peek(), outputBuffer_.readableBytes());
            if (n > 0)
            {
                outputBuffer_.retrieve(n);
                if (outputBuffer_.readableBytes() == 0)
                {
                    channel_->disableWriting();
                    if (writeCompleteCallback_)
                        loop_->queueInLoop([guard = shared_from_this()] () {
                            guard->writeCompleteCallback_(guard);
                        });
                    //之前调用过shutdown()但因为数据没发送完而没有执行，现在要再次shutdown
                    if (state_.load() == Disconnecting)
                        shutdownInLoop();
                }
            }
            else
            {
                // TODO handle error
            }
        }
    }

    void TcpConnection::handleClose()
    {
        loop_->assertInLoopThread();
        assert(state_.load() == Connected || state_.load() == Disconnecting);
        setState(Disconnected);
        channel_->disableAll();
        channel_->remove();

        TcpConnectionPtr guardThis(shared_from_this());
        connectionCallback_(guardThis);
        closeCallback_(guardThis);
    }

    void TcpConnection::handleError()
    {
        int err = socket_.error();
        // TODO handle error
    }

    void TcpConnection::setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

    void TcpConnection::setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void TcpConnection::setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    reactor::EventLoop* TcpConnection::getLoop() const { return loop_; }

    const Inet4Address& TcpConnection::localAddress() const { return localAddr_; }

    const Inet4Address& TcpConnection::peerAddress() const { return peerAddr_; }

    bool TcpConnection::connected() const { return state_.load() == Connected; }

    bool TcpConnection::disconnected() const { return state_.load() == Disconnected; }

    void  TcpConnection::setState(StateE s) { state_.store(s); }

    void TcpConnection::setTcpNoDelay(bool enable) { socket_.setTcpNoDelay(enable); }
}