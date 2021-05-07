#include "ffxiv/stream_handler.h"
#include "tcp_table.h"
#include "utils.h"

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <pcap/pcap.h>
#include <tins/tins.h>
#include <tins/tcp_ip/stream_follower.h>

/**
 * @brief Configures the global spdlog logger.
 */
static void setup_logging()
{
    // Replace the default logger with one that logs to stderr
    spdlog::set_default_logger(spdlog::stderr_color_mt("<UNUSED>"));
    spdlog::set_default_logger(spdlog::stderr_color_mt(""));

#ifdef NDEBUG
    spdlog::set_level(spdlog::level::warn);
#else
    spdlog::set_level(spdlog::level::debug);
    spdlog::trace("Compiled in Debug configuration (NDEBUG not defined)");
#endif

    spdlog::cfg::load_env_levels();
    spdlog::trace(pcap_lib_version());
}

/**
 * @brief Gets a new packet sniffer for the default network interface.
 */
static Tins::Sniffer get_sniffer()
{
    static const Tins::SnifferConfiguration sniffer_config = []() {
        Tins::SnifferConfiguration cfg;

        cfg.set_direction(pcap_direction_t::PCAP_D_INOUT);
        cfg.set_filter("tcp and src portrange 49152-65535 and dst portrange 49152-65535");
        cfg.set_immediate_mode(true);
        cfg.set_promisc_mode(false);
        cfg.set_timeout(100);

        return cfg;
    }();

    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();

    spdlog::info(
        "Sniffing on interface: {} ({}) (HW: {})",
        gunblade::to_utf8(iface.friendly_name()),
        iface.name(),
        iface.hw_address().to_string());

    return Tins::Sniffer(iface.name(), sniffer_config);
}

int main()
{
    setup_logging();

    // Set up a new stream follower to do TCP stream reassembly
    Tins::TCPIP::StreamFollower follower;
    gunblade::ffxiv::setup_follower(follower);

    // Sniff packets forever
    Tins::Sniffer sniffer = get_sniffer();
    sniffer.sniff_loop([&follower](Tins::PDU& packet) {
        follower.process_packet(packet);
        return true;
    });

    return 0;
}
