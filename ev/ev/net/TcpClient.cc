//
// Created by zr on 23-3-5.
//

#include "TcpClient.h"
#include "Socket.h"
#include "Connector.h"
#include "TcpConnection.h"
#include <iostream>

namespace ev::net
{
    TcpClient::TcpClient(reactor::EventLoop* loop, const Inet4Address& serverAddr):
        loop_(loop),
        connector_(new Connector(loop, serverAddr)),
        disconnected_(true),
        latch_(2)
    {
        connector_->setNewConnectionCallback([this] (Socket socket) {
            newConnection(std::move(socket));
        });
    }

    TcpClient::~TcpClient()
    {
        TcpConnectionPtr conn;
        {
            std::lock_guard<std::mutex> lg(mu);
            conn = connection_;
        }
        if(conn)
        {
            loop_->runInLoop([this, &conn] () {
                conn->setCloseCallback([] (const TcpConnectionPtr& conn) {});
                conn->setConnectionCallback([] (const TcpConnectionPtr& conn) {});
                latch_.countDown();
            });
        }
        else latch_.countDown();
        loop_->runInLoop([this] () {
            connector_->stop();
            latch_.countDown();
        });
        latch_.wait();
        if(conn) conn->forceClose();
    }

    void TcpClient::connect()
    {
        if(disconnected_.exchange(false))
            connector_->start();
    }

    void TcpClient::disconnect()
    {
        if(!disconnected_.load())
        {
            std::lock_guard<std::mutex> lg(mu);
            if(connection_)
                connection_->shutdown();
        }
    }

    void TcpClient::send(const void* message, int len)
    {
        if(!disconnected_.load())
        {
            std::lock_guard<std::mutex> lg(mu);
            if(connection_)
                connection_->send(message, len);
        }
    }

    void TcpClient::send(Buffer& message)
    {
        if(!disconnected_.load())
        {
            std::lock_guard<std::mutex> lg(mu);
            if(connection_)
                connection_->send(message);
        }
    }

    void TcpClient::setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;}

    void TcpClient::setMessageCallback(const MessageCallback& cb) {messageCallback_ = cb;}

    void TcpClient::setWriteCompleteCallback(const WriteCompleteCallback& cb) {writeCompleteCallback_ = cb;}

    void TcpClient::newConnection(Socket socket)
    {
        loop_->assertInLoopThread();
        Inet4Address localAddr = socket.localAddress();
        Inet4Address peerAddr = socket.peerAddress();
        TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, std::move(socket),
                                                                localAddr, peerAddr);
        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setWriteCompleteCallback(writeCompleteCallback_);
        conn->setCloseCallback([this] (const TcpConnectionPtr& conn) { removeConnection(conn);});
        {
            std::lock_guard<std::mutex> lg(mu);
            connection_ = conn;
        }
        conn->setTcpNoDelay(true);
        conn->connectEstablished();
    }

    void TcpClient::removeConnection(const TcpConnectionPtr& conn)
    {
        loop_->assertInLoopThread();
        disconnected_.store(true);
        {
            std::lock_guard<std::mutex> lg(mu);
            connection_.reset();
        }
    }
}