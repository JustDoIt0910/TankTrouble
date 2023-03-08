//
// Created by zr on 23-3-1.
//

#ifndef EV_INET_ADDRESS_H
#define EV_INET_ADDRESS_H
#include <string_view>
#include <string>
#include <netinet/in.h>

namespace ev::net
{
    class Inet4Address
    {
    public:
        Inet4Address(std::string_view ip, uint16_t port);
        Inet4Address(uint16_t port, bool loopBackOnly);
        explicit Inet4Address(const sockaddr_in& addr);
        std::string toIpPort();
        std::string toIp();
        [[nodiscard]] uint16_t port() const;
        [[nodiscard]] sa_family_t family() const;
        [[nodiscard]] const struct sockaddr* getSockAddr() const;
        void setSockAddr(const struct sockaddr* addr);
        static bool resolve(std::string_view name, Inet4Address& address);

    private:
        sockaddr_in addr_;
    };
}

#endif //EV_INET_ADDRESS_H
