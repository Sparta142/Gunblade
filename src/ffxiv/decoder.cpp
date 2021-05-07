#include "decoder.h"

#include <algorithm>  // copy, search
#include <array>      // array
#include <bit>        // bit_cast
#include <utility>    // move
#include <iterator>   // distance

// clang-format off
static constexpr std::array<uint8_t, 16> magic_number {
    0x52, 0x52, 0xa0, 0x41,  // 0x41a05252
    0xff, 0x5d, 0x46, 0xe2,  // 0xe2465dff
    0x7f, 0x2a, 0x64, 0x4d,  // 0x4d642a7f
    0x7b, 0x99, 0xc4, 0x75   // 0x75c4997b
};

static constexpr std::array<uint8_t, 16> magic_number_keepalive {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
// clang-format on

template <typename T, typename InputIterator>
static T read_struct(InputIterator begin)
{
    std::array<unsigned char, sizeof(T)> buffer;
    std::copy(begin, begin + buffer.size(), buffer.data());
    return std::bit_cast<T>(buffer);
}

namespace gunblade
{
    std::optional<Bundle> FinalFantasyDecoder::next_bundle()
    {
        // Search for the IPC bundle magic number in the buffer
        auto iter =
            std::search(data_.cbegin(), data_.cend(), magic_number.cbegin(), magic_number.cend());

        // Did we find the magic number in the buffer?
        if (iter == data_.cend())
        {
            // No - try searching for the keepalive bundle magic number instead
            iter = std::search(
                data_.cbegin(),
                data_.cend(),
                magic_number_keepalive.cbegin(),
                magic_number_keepalive.cend());

            // Did we find that instead?
            if (iter == data_.cend())
            {
                // No - just return nothing. Don't trim the buffer because
                // it might end with the start of a valid bundle.
                return std::nullopt;
            }
        }

        const auto remaining_length = std::distance(iter, data_.cend());

        // Is there enough data left to read an entire bundle header?
        if (remaining_length < sizeof(Bundle::Header))
        {
            // No - trim everything in the buffer before the magic number,
            // then return nothing.
            data_.erase(data_.cbegin(), iter);
            return std::nullopt;
        }

        // Read the *entire* header from the buffer
        const auto bundle_header = read_struct<Bundle::Header>(iter);

        // Is there enough data left to read the entire bundle? (not just its header)
        if (remaining_length < bundle_header.length)
        {
            // No - trim everything in the buffer before the magic number,
            // then return nothing.
            data_.erase(data_.cbegin(), iter);
            return std::nullopt;
        }

        // Get the bounds of the bundle payload
        const auto payload_begin = iter + sizeof(bundle_header);
        const auto payload_end = iter + bundle_header.length;

        // Create the bundle and set up its header and payload
        Bundle bundle;
        bundle.header = std::move(bundle_header);
        bundle.payload.assign(payload_begin, payload_end);

        // Trim everything in the buffer before the magic number
        data_.erase(data_.cbegin(), payload_end);

        // Return the complete bundle
        return bundle;
    }
}  // namespace gunblade
