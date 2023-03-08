//
// Created by zr on 23-3-4.
//

#ifndef EV_TCP_CONNECTION_H
#define EV_TCP_CONNECTION_H
#include <memory>
#include <string>
#include <functional>
#include <atomic>
#include "Inet4Address.h"
#include "Buffer.h"
#include "Socket.h"
#include "utils/noncopyable.h"
#include "utils/Timestamp.h"
#include "reactor/Channel.h"
#include "Types.h"

namespace ev::reactor { class EventLoop; }

namespace ev::net
{
    class TcpConnection : public noncopyable,
                          public std::enable_shared_from_this<TcpConnection>
    {
    public:
        const static size_t DefaultHighWaterMark;

        TcpConnection(reactor::EventLoop* loop,
                      Socket&& socket,
                      const Inet4Address& localAddr,
                      const Inet4Address& peerAddr);
        ~TcpConnection();

        reactor::EventLoop* getLoop() const;
        const Inet4Address& localAddress() const;
        const Inet4Address& peerAddress() const;
        bool connected() const;
        bool disconnected() const;

        void send(const void* message, int len);
        void send(Buffer& message);
        void shutdown();
        void forceClose();

        void startRead();
        void stopRead();

        void setConnectionCallback(const ConnectionCallback& cb);
        void setMessageCallback(const MessageCallback& cb);
        void setWriteCompleteCallback(const WriteCompleteCallback& cb);
        void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark);
        void setCloseCallback(const CloseCallback& cb);

        void connectEstablished();
        void setTcpNoDelay(bool enable);

    private:
        enum StateE { Disconnected, Connecting, Connected, Disconnecting };
        void setState(StateE s);

        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const void* data, size_t len);
        void sendInLoop(const std::string& str);
        void shutdownInLoop();
        void forceCloseInLoop();
        void startReadInLoop();
        void stopReadInLoop();

        reactor::EventLoop* loop_;
        std::atomic<StateE> state_;
        Socket socket_;
        std::unique_ptr<reactor::Channel> channel_;
        const Inet4Address localAddr_;
        const Inet4Address peerAddr_;
        Buffer inputBuffer_;
        Buffer outputBuffer_;
        size_t highWaterMark_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;
        CloseCallback closeCallback_;
    };
}


#endif //EV_TCP_CONNECTION_H
