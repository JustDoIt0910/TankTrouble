//
// Created by zr on 23-3-5.
//

#ifndef EV_TCP_CLIENT_H
#define EV_TCP_CLIENT_H
#include "Inet4Address.h"
#include "utils/noncopyable.h"
#include "utils/CountDownLatch.h"
#include "Types.h"
#include <memory>
#include <atomic>
#include <mutex>

namespace ev::reactor { class EventLoop; }

namespace ev::net
{
    class TcpClient : public noncopyable
    {
    public:
        TcpClient(reactor::EventLoop* loop, const Inet4Address& serverAddr);
        ~TcpClient();
        void connect();
        void disconnect();
        void send(const void* message, int len);
        void send(Buffer& message);
        void setConnectionCallback(const ConnectionCallback& cb);
        void setMessageCallback(const MessageCallback& cb);
        void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    private:
        void newConnection(Socket socket);
        void removeConnection(const TcpConnectionPtr& conn);

        reactor::EventLoop* loop_;
        ConnectorPtr connector_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        std::mutex mu;
        TcpConnectionPtr connection_;
        std::atomic_bool disconnected_;
        CountDownLatch latch_;
    };
}

#endif //EV_TCP_CLIENT_H
