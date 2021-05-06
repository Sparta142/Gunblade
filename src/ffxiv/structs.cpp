#include "structs.h"

#include <array>
#include <stdexcept>    // runtime_error
#include <type_traits>  // is_standard_layout_v
#include <utility>      // copy

#pragma warning(push)
#pragma warning(disable : 4068)
#include <gzip/decompress.hpp>
#pragma warning(pop)

template <typename T, typename InputIterator>
static T read_struct(InputIterator begin)  // TODO: Deduplicate
{
    std::array<unsigned char, sizeof(T)> buffer;
    std::copy(begin, begin + buffer.size(), buffer.data());
    return std::bit_cast<T>(buffer);
}

template <typename Enum>
static constexpr auto underlying_cast(Enum value) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(value);
}

namespace machinist::ffxiv
{
    std::vector<char> Bundle::decompressed_payload() const
    {
        switch (header.compression)
        {
            case Compression::NONE:
                return payload;

            case Compression::ZLIB:
            {
                const auto inflated = gzip::decompress(payload.data(), payload.size());
                return std::vector(inflated.cbegin(), inflated.cend());
            }

            default:
                throw std::runtime_error("Unknown bundle compression");
        }
    }

    std::vector<Segment> Bundle::segments() const
    {
        const auto decompressed = decompressed_payload();
        auto iter = decompressed.cbegin();

        std::vector<Segment> segs;
        segs.reserve(header.message_count);

        for (auto i = 0; i < header.message_count; ++i)
        {
            const auto seg_header = read_struct<Segment::Header>(iter);
            const auto seg_payload_begin = iter + sizeof(seg_header);
            const auto seg_payload_end = iter + seg_header.size;
            std::vector<char> seg_payload(seg_payload_begin, seg_payload_end);

            iter = seg_payload_end;
            segs.emplace_back(seg_header, seg_payload);
        }

        return segs;
    }

    void to_json(nlohmann::json& j, const Bundle& bundle)
    {
        // clang-format off
        j = nlohmann::json{
            {"epoch", bundle.header.epoch},
            {"segments", bundle.segments()}
        };
        // clang-format on
    }

    void to_json(nlohmann::json& j, const Segment& segment)
    {
        // clang-format off
        j = nlohmann::json{
            {"source", segment.header.source},
            {"target", segment.header.target},
            {"type", underlying_cast(segment.header.type)},
            {"payload", {}}  // TODO
        };
        // clang-format on
    }

    static_assert(std::is_standard_layout_v<Bundle::Header>);
    static_assert(std::is_standard_layout_v<Segment::Header>);
    static_assert(std::is_standard_layout_v<IPC::Header>);
    static_assert(std::is_standard_layout_v<ClientKeepAlive>);
    static_assert(std::is_standard_layout_v<ServerKeepAlive>);
    static_assert(sizeof(Bundle::Header) == 40);
    static_assert(sizeof(Segment::Header) == 16);
    static_assert(sizeof(IPC::Header) == 16);
    static_assert(sizeof(ClientKeepAlive) == 8);
    static_assert(sizeof(ServerKeepAlive) == 8);

}  // namespace machinist::ffxiv
