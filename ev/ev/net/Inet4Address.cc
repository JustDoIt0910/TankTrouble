//
// Created by zr on 23-3-1.
//

#include "Inet4Address.h"
#include <string.h>
#include <arpa/inet.h>
#include <cassert>
#include <netdb.h>
#include "Endian.h"

namespace ev::net
{
    Inet4Address::Inet4Address(std::string_view ip, uint16_t port)
    {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        inet_pton(AF_INET, ip.data(), &addr_.sin_addr);
        addr_.sin_port = hostToNetwork16(port);
    }

    Inet4Address::Inet4Address(uint16_t port, bool loopBackOnly)
    {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = hostToNetwork16(port);
        if(loopBackOnly)
            addr_.sin_addr.s_addr = hostToNetwork32(INADDR_LOOPBACK);
        else
            addr_.sin_addr.s_addr = hostToNetwork32(INADDR_ANY);
    }

    Inet4Address::Inet4Address(const sockaddr_in& addr): addr_(addr) {}

    std::string Inet4Address::toIpPort()
    {
        char buf[32];
        size_t bufSize = sizeof(buf);
        bzero(buf, bufSize);
        inet_ntop(AF_INET, &addr_.sin_addr, buf, bufSize);
        size_t end = ::strlen(buf);
        assert(end < bufSize);
        snprintf(buf + end, bufSize - end, ":%u", networkToHost16(addr_.sin_port));
        return buf;
    }

    std::string Inet4Address::toIp()
    {
        char buf[32];
        size_t bufSize = sizeof(buf);
        bzero(buf, bufSize);
        inet_ntop(AF_INET, &addr_.sin_addr, buf, bufSize);
        return buf;
    }

    uint16_t Inet4Address::port() const {return networkToHost16(addr_.sin_port);}

    sa_family_t Inet4Address::family() const {return addr_.sin_family;}

    const struct sockaddr* Inet4Address::getSockAddr() const
    {
        return static_cast<const struct sockaddr*>(static_cast<const void*>(&addr_));
    }

    void Inet4Address::setSockAddr(const struct sockaddr* addr)
    {
        addr_ = *static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
    }

    static __thread char resolveBuf[8192];

    bool Inet4Address::resolve(std::string_view name, Inet4Address& address)
    {
        struct hostent ent{};
        struct hostent* res = nullptr;
        int h_errorno = 0;
        int ret = gethostbyname_r(name.data(), &ent, resolveBuf, sizeof(resolveBuf), &res, &h_errorno);
        if(ret == 0 && res != nullptr)
        {
            assert(res->h_addrtype == AF_INET && res->h_length == 4);
            address.addr_.sin_addr = *reinterpret_cast<struct in_addr*>(res->h_addr);
        }
        else
        {
            if(ret)
            {
                // TODO handle error
            }
            return false;
        }
        return true;
    }
}