set(REACTOR_SRC Inet4Address.cc
        Socket.cc
        Buffer.cc
        TcpConnection.cc
        Connector.cc
        TcpClient.cc
        ../utils/CountDownLatch.cc)

add_library(net ${REACTOR_SRC})