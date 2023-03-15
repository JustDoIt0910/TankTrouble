//
// Created by zr on 23-3-14.
//

#ifndef EV_TCPSERVER_H
#define EV_TCPSERVER_H
#include "Inet4Address.h"
#include "utils/noncopyable.h"
#include "Types.h"
#include <map>
#include <atomic>

namespace ev::reactor
{
    class EventLoop;
    class Channel;
}
using namespace ev::reactor;

namespace ev::net
{
    class EventLoopThreadPool;
    class Acceptor;
    class TcpServer : public noncopyable
    {
    public:
        TcpServer(EventLoop* loop, const Inet4Address& listenAddr);
        ~TcpServer();

        [[nodiscard]] std::string ipPort() const;
        [[nodiscard]] EventLoop* getLoop() const;

        void setThreadNum(int numThreads);
        std::shared_ptr<EventLoopThreadPool> threadPool();
        void start();

        void setConnectionCallback(const ConnectionCallback& cb);
        void setMessageCallback(const MessageCallback& cb);
        void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    private:
        void newConnection(Socket socket);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);

        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop* loop_;
        const std::string ipPort_;
        std::unique_ptr<Acceptor> acceptor_;
        std::shared_ptr<EventLoopThreadPool> threadPool_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        std::atomic_bool started_;
        ConnectionMap connections_;
    };
}

#endif //EV_TCPSERVER_H
