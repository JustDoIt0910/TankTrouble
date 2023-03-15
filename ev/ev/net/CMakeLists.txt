set(REACTOR_SRC Inet4Address.cc
        Socket.cc
        Buffer.cc
        TcpConnection.cc
        Connector.cc
        Acceptor.cc
        TcpClient.cc
        EventLoopThread.cc
        EventLoopThreadPool.cc
        TcpServer.cc
        ../utils/CountDownLatch.cc)

add_library(net ${REACTOR_SRC})