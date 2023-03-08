//
// Created by zr on 23-3-5.
//
#include "net/TcpClient.h"
#include "utils/Timestamp.h"
#include "reactor/EventLoop.h"
#include "net/TcpConnection.h"
#include <iostream>

using namespace ev::reactor;
using namespace ev::net;

int main()
{
    EventLoop loop;
    TcpClient* client = new TcpClient(&loop, Inet4Address("127.0.0.1", 9999));
    client->setConnectionCallback([] (const TcpConnectionPtr& conn) {
        if(conn->connected())
        {
            std::cout << "connected" << std::endl;
            conn->send("hello", 5);
        }
        else
            std::cout << "disconnected" << std::endl;

    });
    client->setMessageCallback([] (const TcpConnectionPtr&, Buffer* buf, ev::Timestamp) {
        std::cout << buf->retrieveAllAsString() << std::endl;
    });
    client->connect();
    loop.runAfter(3.0, [client] () {delete client;});
    loop.loop();
    return 0;
}