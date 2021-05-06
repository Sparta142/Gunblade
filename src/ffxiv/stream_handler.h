#pragma once

#include <tins/tcp_ip/stream_follower.h>

namespace machinist::ffxiv
{
    void setup_follower(Tins::TCPIP::StreamFollower& follower);
}  // namespace machinist::ffxiv
