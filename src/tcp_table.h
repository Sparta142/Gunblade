#pragma once

#include <variant>
#include <vector>

#include <tins/ip_address.h>
#include <tins/ipv6_address.h>

namespace machinist
{
    using IPAddress = std::variant<Tins::IPv4Address, Tins::IPv6Address>;

    enum class TcpState : unsigned long
    {
        UNKNOWN = 0,
        CLOSED = 1,
        LISTEN = 2,
        SYN_SENT = 3,
        SYN_RCVD = 4,
        ESTABLISHED = 5,
        FIN_WAIT1 = 6,
        FIN_WAIT2 = 7,
        CLOSE_WAIT = 8,
        CLOSING = 9,
        LAST_ACK = 10,
        TIME_WAIT = 11,
        DELETE_TCB = 12
    };

    struct ConnectionInfo
    {
        IPAddress local_addr;
        unsigned short local_port;

        IPAddress remote_addr;
        unsigned short remote_port;

        TcpState state;
        unsigned long pid;

        inline bool is_v6() const noexcept
        {
            return std::holds_alternative<Tins::IPv6Address>(local_addr);
        }

        inline const Tins::IPv4Address local_addr_v4() const
        {
            return std::get<Tins::IPv4Address>(local_addr);
        }

        inline const Tins::IPv6Address local_addr_v6() const
        {
            return std::get<Tins::IPv6Address>(local_addr);
        }

        inline const Tins::IPv4Address remote_addr_v4() const
        {
            return std::get<Tins::IPv4Address>(remote_addr);
        }

        inline const Tins::IPv6Address remote_addr_v6() const
        {
            return std::get<Tins::IPv6Address>(remote_addr);
        }
    };

    std::vector<ConnectionInfo> get_tcp_table();
}  // namespace machinist
