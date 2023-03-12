//
// Created by zr on 23-3-2.
//

#include "Socket.h"
#include <strings.h>
#include <unistd.h>
#include <netinet/tcp.h>

namespace ev::net
{
    Socket::Socket(sa_family_t family)
    {
        fd_ = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if(fd_ < 0)
            abort();
    }

    Socket::Socket(int sockFd): fd_(sockFd) {}

    Socket::Socket(): Socket(-1) {}

    Socket::Socket(Socket&& socket) noexcept
    {
        fd_ = socket.fd();
        socket.fd_ = -1;
    }

    Socket& Socket::operator=(Socket&& socket) noexcept
    {
        if(fd_ >= 0)
            ::close(fd_);
        fd_ = socket.fd();
        socket.fd_ = -1;
        return *this;
    }

    Socket::~Socket()
    {
        if(fd_ >= 0)
            ::close(fd_);
    }

    void Socket::setReuseAddr(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::setReusePort(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::setTcpNoDelay(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            int a;
            // TODO handle error
        }
    }

    void Socket::setKeepAlive(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::bind(const Inet4Address& addr) const
    {
        if(::bind(fd_, addr.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr))) < 0)
            abort();
    }

    void Socket::listen() const
    {
        if(::listen(fd_, SOMAXCONN) < 0)
            abort();
    }

    Socket Socket::accept(Inet4Address &address) const
    {
        struct sockaddr addr{};
        bzero(&addr, sizeof(addr));
        socklen_t len = static_cast<socklen_t>(sizeof(addr));
        int fd = ::accept4(fd_, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(fd < 0)
        {
            int savedErrno = errno;
            switch (savedErrno)
            {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO:
                case EPERM:
                case EMFILE:
                    errno = savedErrno;
                    break;
                default: abort();
            }
        }
        else
            address.setSockAddr(&addr);
        return Socket(fd);
    }

    int Socket::connect(const Inet4Address& serverAddr) const
    {
        return ::connect(fd_, serverAddr.getSockAddr(),
                         static_cast<socklen_t>(sizeof(struct sockaddr)));
    }

    ssize_t Socket::write(const void* data, size_t len) const {return ::write(fd_, data, len);}

    int Socket::fd() const {return fd_;}

    Inet4Address Socket::localAddress() const
    {
        struct sockaddr_in localAddr{};
        socklen_t len = static_cast<socklen_t>(sizeof(localAddr));
        bzero(&localAddr, len);
        if(::getsockname(fd_,static_cast<struct sockaddr*>(static_cast<void*>(&localAddr)),
                &len) < 0)
        {
            // TODO handle error
        }
        return Inet4Address(localAddr);
    }

    Inet4Address Socket::peerAddress() const
    {
        struct sockaddr_in peerAddr{};
        socklen_t len = static_cast<socklen_t>(sizeof(peerAddr));
        bzero(&peerAddr, len);
        if(::getpeername(fd_,static_cast<struct sockaddr*>(static_cast<void*>(&peerAddr)),
                         &len) < 0)
        {
            // TODO handle error
        }
        return Inet4Address(peerAddr);
    }

    int Socket::error() const
    {
        int opt;
        socklen_t len = static_cast<socklen_t>(sizeof(int));
        ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &opt, &len);
        return opt;
    }

    void Socket::shutdownWrite() const
    {
        if(::shutdown(fd_, SHUT_WR) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::close()
    {
        if(fd_ >= 0)
            ::close(fd_);
        fd_ = -1;
    }
}