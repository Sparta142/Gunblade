#include "decoder.h"

#include <algorithm>
#include <array>
#include <bit>
#include <utility>

// clang-format off
static constexpr std::array<uint8_t, 16> magic_number {
    0x52, 0x52, 0xa0, 0x41,
    0xff, 0x5d, 0x46, 0xe2,
    0x7f, 0x2a, 0x64, 0x4d,
    0x7b, 0x99, 0xc4, 0x75
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
        // Search for the magic number in the buffer
        const auto iter =
            std::search(data_.cbegin(), data_.cend(), magic_number.cbegin(), magic_number.cend());

        // Did we find the needle (magic number) in the haystack (buffer)?
        if (iter == data_.cend())
        {
            // No - just return nothing
            return std::nullopt;
        }

        const auto remaining_length = std::distance(iter, data_.cend());

        // Is there enough data left to read an entire bundle header?
        if (remaining_length < sizeof(Bundle::Header))
        {
            // No - erase everything before the magic number, then return nothing
            data_.erase(data_.cbegin(), iter);
            return std::nullopt;
        }

        // Read the *entire* header from the buffer
        const auto bundle_header = read_struct<Bundle::Header>(iter);

        // Is there enough data left to read the entire bundle? (not just its header)
        if (remaining_length < bundle_header.length)
        {
            // No - erase everything before the magic number, then return nothing
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

        // Erase everything in the buffer up to the end of the bundle we just read
        data_.erase(data_.cbegin(), payload_end);

        // Return the complete bundle
        return bundle;
    }
}  // namespace gunblade
