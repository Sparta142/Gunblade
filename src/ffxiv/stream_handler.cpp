#include "../tcp_table.h"
#include "../utils.h"
#include "decoder.h"
#include "stream_handler.h"

#include <iostream>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <tins/ip_address.h>
#include <tins/ipv6_address.h>

#include <nlohmann/json.hpp>

using Tins::TCPIP::Flow;
using Tins::TCPIP::Stream;
using Tins::TCPIP::StreamFollower;

using json = nlohmann::json;
using TerminationReason = Tins::TCPIP::StreamFollower::TerminationReason;
using pid_t = unsigned long;

template <>
struct fmt::formatter<Tins::TCPIP::Stream> final
{
    constexpr auto parse(format_parse_context& ctx)
    {
        if (ctx.begin() != ctx.end())
        {
            // No format string allowed
            throw format_error("invalid format");
        }

        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Tins::TCPIP::Stream& stream, FormatContext& ctx)
    {
        if (stream.is_v6())
        {
            return format_to(
                ctx.out(),
                "{}:{} <-> {}:{}",
                stream.client_addr_v6().to_string(),
                stream.client_port(),
                stream.server_addr_v6().to_string(),
                stream.server_port());
        }
        else
        {
            return format_to(
                ctx.out(),
                "{}:{} <-> {}:{}",
                stream.client_addr_v4().to_string(),
                stream.client_port(),
                stream.server_addr_v4().to_string(),
                stream.server_port());
        }
    }
};

static bool stream_matches(const machinist::ConnectionInfo& info, const Stream& stream)
{
    // Are they even the same address family?
    if (stream.is_v6() != info.is_v6())
    {
        return false;
    }

    bool local_port_is_client =
        info.local_port == stream.client_port() && info.remote_port == stream.server_port();
    bool local_port_is_server =
        info.local_port == stream.server_port() && info.remote_port == stream.client_port();

    // Do the port numbers even match?
    if (!local_port_is_client && !local_port_is_server)
    {
        return false;
    }

    if (info.is_v6())
    {
        // Compare IPv6 addresses
        return (local_port_is_client && info.local_addr_v6() == stream.client_addr_v6() &&
                info.remote_addr_v6() == stream.server_addr_v6()) ||
               (local_port_is_server && info.local_addr_v6() == stream.server_addr_v6() &&
                info.remote_addr_v6() == stream.client_addr_v6());
    }
    else
    {
        // Compare IPv4 addresses
        return (local_port_is_client && info.local_addr_v4() == stream.client_addr_v4() &&
                info.remote_addr_v4() == stream.server_addr_v4()) ||
               (local_port_is_server && info.local_addr_v4() == stream.server_addr_v4() &&
                info.remote_addr_v4() == stream.client_addr_v4());
    }
}

static inline bool is_ffxiv_pid(pid_t pid)
{
    return machinist::get_process_name(pid) == "ffxiv_dx11.exe";
}

static std::optional<pid_t> get_ffxiv_pid(const Stream& stream)
{
    // Look through all TCP connections
    for (const auto& info : machinist::get_tcp_table())
    {
        if (stream_matches(info, stream) && is_ffxiv_pid(info.pid))
        {
            return info.pid;
        }
    }

    // Didn't find a connection that both matches and is from an FFXIV process
    return std::nullopt;
}

template <typename T>
static constexpr bool is_same(const T& left, const T& right)
{
    return std::addressof(left) == std::addressof(right);
}

static std::string addr_to_string(const Flow& flow)
{
    if (flow.is_v6())
    {
        return flow.dst_addr_v6().to_string();
    }
    else
    {
        return flow.dst_addr_v4().to_string();
    }
}

class DataCallback final
{
public:
    explicit DataCallback(Flow& flow, std::string name, pid_t pid)
        : flow_(flow), name_(std::move(name)), pid_(pid)
    {
        // Do nothing
    }

    void operator()(Stream& stream)
    {
        const auto& payload = flow_.payload();
        decoder_.feed_data(payload.cbegin(), payload.cend());

        // Handle all bundles using the decoder
        std::optional<machinist::Bundle> bundle;
        while ((bundle = decoder_.next_bundle()).has_value())
        {
            const auto& source_flow =
                is_same(flow_, stream.client_flow()) ? stream.server_flow() : stream.client_flow();

            // clang-format off
            json obj = {
                {"connection", {
                    {"source", {
                        {"host", addr_to_string(source_flow)},
                        {"port", source_flow.dport()}
                    }},
                    {"destination", {
                        {"host", addr_to_string(flow_)},
                        {"port", flow_.dport()}
                    }},
                }},
                {"processId", pid_},
                {"bundle", *bundle}
            };
            // clang-format on

            // Write the JSON object to stdout - https://jsonlines.org/
            std::cout << obj.dump() << std::endl;
        }

        // Don't let the decoder buffer data forever, stop it once
        // it reaches a critical length of 2 max-sized bundles.
        if (decoder_.size() > (2 * machinist::ffxiv::Bundle::max_length))
        {
            spdlog::warn("Flow {} buffered too much data without a bundle, ignoring it", name_);

            flow_.ignore_data_packets();
            decoder_.clear();
        }
    }

private:
    machinist::FinalFantasyDecoder decoder_;
    Flow& flow_;

    const std::string name_;
    const pid_t pid_;
};

namespace machinist::ffxiv
{
    static void on_new_stream(Stream& stream)
    {
        auto pid = get_ffxiv_pid(stream);

        if (!pid.has_value())
        {
            // The stream doesn't refer to a connection opened by an FFXIV client
            stream.ignore_client_data();
            stream.ignore_server_data();
            return;
        }

        spdlog::info("FFXIV stream detected: {} (pid = {})", stream, *pid);

        stream.client_data_callback(DataCallback(stream.client_flow(), "client", *pid));
        stream.server_data_callback(DataCallback(stream.server_flow(), "server", *pid));
    }

    static void on_stream_termination(Stream& stream, TerminationReason reason)
    {
        // TODO: Probably not necessary
        spdlog::warn("Stream terminated: {} (why: {})", stream, reason);
    }

    void setup_follower(StreamFollower& follower)
    {
        follower.follow_partial_streams(true);
        follower.new_stream_callback(on_new_stream);
        follower.stream_termination_callback(on_stream_termination);
    }
}  // namespace machinist::ffxiv
