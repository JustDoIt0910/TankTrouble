//
// Created by zr on 23-3-5.
//

#ifndef EV_TYPES_H
#define EV_TYPES_H
#include <functional>
#include <memory>

namespace ev { class Timestamp; }

namespace ev::net
{
    class Socket;
    class TcpConnection;
    class Connector;
    class Buffer;

    // Connector
    typedef std::shared_ptr<Connector> ConnectorPtr;
    typedef std::function<void (Socket socket)> NewConnectionCallback;

    // TcpConnection
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
    typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
    typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
    typedef std::function<void (const TcpConnectionPtr&,
                                Buffer*,
                                Timestamp)> MessageCallback;
}

#endif //EV_TYPES_H
