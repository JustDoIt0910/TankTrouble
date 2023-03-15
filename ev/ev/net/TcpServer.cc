//
// Created by zr on 23-3-14.
//

#include "TcpServer.h"
#include "reactor/EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"

using namespace ev::reactor;

namespace ev::net
{
    TcpServer::TcpServer(EventLoop* loop, const Inet4Address& listenAddr):
        loop_(loop),
        ipPort_(listenAddr.toIpPort()),
        acceptor_(new Acceptor(loop_, listenAddr, true)),
        threadPool_(new EventLoopThreadPool(loop_)),
        started_(false)
    {
        acceptor_->setNewConnectionCallback([this] (Socket socket) {
            newConnection(std::move(socket));
        });
    }

    TcpServer::~TcpServer() = default;

    std::string TcpServer::ipPort() const {return ipPort_;}

    EventLoop* TcpServer::getLoop() const {return loop_;}

    void TcpServer::setThreadNum(int numThreads)
    {
        if(numThreads <= 0)
            return;
        threadPool_->setThreadNum(numThreads);
    }

    std::shared_ptr<EventLoopThreadPool> TcpServer::threadPool() {return threadPool_;}

    void TcpServer::setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;}

    void TcpServer::setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;}

    void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb) {writeCompleteCallback_ = cb;}

    void TcpServer::start()
    {
        if (!started_.exchange(true))
        {
            threadPool_->start();
            loop_->runInLoop([this] () {acceptor_->listen();});
        }
    }

    void TcpServer::newConnection(Socket socket)
    {
        loop_->assertInLoopThread();
        EventLoop* ioLoop = threadPool_->getNextLoop();
        Inet4Address localAddr = socket.localAddress();
        Inet4Address peerAddr = socket.peerAddress();
        TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                                std::move(socket),
                                                localAddr,
                                                peerAddr));
        std::string connId = peerAddr.toIpPort();
        connections_[connId] = conn;
        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setWriteCompleteCallback(writeCompleteCallback_);
        conn->setCloseCallback([this] (const TcpConnectionPtr& conn) { removeConnection(conn);});
        ioLoop->runInLoop([conn] () { conn->connectEstablished();});
    }

    void TcpServer::removeConnection(const TcpConnectionPtr& conn)
    {
        loop_->runInLoop([this, conn] () {
            removeConnectionInLoop(conn);
        });
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
    {
        loop_->assertInLoopThread();
        size_t n = connections_.erase(conn->peerAddress().toIpPort());
        assert(n == 1);
    }
}

