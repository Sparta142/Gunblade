#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

namespace gunblade::ffxiv
{
    enum class Compression : std::uint8_t
    {
        /** @brief Specifies that a bundle payload is not compressed. */
        NONE = 0x00,

        /**
         * @brief Specifies that a bundle payload is compressed
         * using Zlib compression.
         */
        ZLIB = 0x01
    };

    enum class SegmentType : std::uint16_t
    {
        IPC = 3,
        CLIENT_KEEPALIVE = 7,
        SERVER_KEEPALIVE = 8
    };

    struct Bundle final
    {
#pragma pack(push, 1)
        struct Header final
        {
            /** @brief Bytes 0-3 of the magic number. Usually 0x41a05252. */
            std::uint32_t magic_0;

            /** @brief Bytes 4-7 of the magic number. Usually 0xe2465dff. */
            std::uint32_t magic_1;

            /** @brief Bytes 8-11 of the magic number. Usually 0x4d642a7f. */
            std::uint32_t magic_2;

            /** @brief Bytes 12-15 of the magic number. Usually 0x75c4997b. */
            std::uint32_t magic_3;

            /** @brief The number of milliseconds since the Unix epoch time. */
            std::uint64_t epoch;

            /** @brief The total length of the bundle, including this header. */
            std::uint16_t length;

            /** @brief Unknown value. Usually 0x0000. */
            std::uint16_t unknown_1;

            /** @brief The connection type. Usually 0x0000. */
            std::uint16_t connection_type;

            /** @brief The number of segments in the bundle. */
            std::uint16_t message_count;

            /** @brief The encoding of the bundle payload. */
            std::uint8_t encoding;

            /** @brief The type of compression used for the bundle payload. */
            Compression compression;

            /** @brief Unknown value. Usually 0x0000. */
            std::uint16_t unknown_2;

            /** @brief Unknown value. Usually 0x0000. */
            std::uint16_t unknown_3;

            /** @brief Unknown value. Usually 0x0000. */
            std::uint16_t unknown_4;

            /** @return The length of the bundle payload. */
            inline decltype(length) payload_length() const noexcept
            {
                return length - sizeof(*this);
            }

            /** @return Whether the bundle payload is compressed. */
            inline bool is_compressed() const noexcept
            {
                return compression != Compression::NONE;
            }
        };
#pragma pack(pop)

        static constexpr std::size_t max_length =
            std::numeric_limits<decltype(Header::length)>::max();

        /** @brief The bundle header (metadata). */
        Header header;

        /** @brief The (possibly compressed) bundle payload. */
        std::vector<char> payload;

        /** @returns The decompressed bundle payload. */
        std::vector<char> decompressed_payload() const;

        std::vector<struct Segment> segments() const;
    };

    struct IPC final
    {
#pragma pack(push, 1)
        struct Header final
        {
            /** @brief The magic number. Usually 0x0014. */
            std::uint16_t magic;

            /**
             * @brief The type of IPC being executed.
             *
             * @note The same type does not necessarily refer to the same
             * IPC when comparing across different FINAL FANTASY XIV patches.
             */
            std::uint16_t type;

            /** @brief Unknown value. Usually 0x0000. */
            std::uint16_t unknown_1;

            /** @brief The ID of the connected FINAL FANTASY XIV server? */
            std::uint16_t server_id;

            /** @brief The number of seconds since the Unix epoch time. */
            std::uint32_t epoch;

            /** @brief Unknown value. Usually 0x00000000. */
            std::uint32_t unknown_2;
        };
#pragma pack(pop)

        Header header;

        std::vector<char> data;
    };

#pragma pack(push, 1)
    template <bool IsClient>
    struct KeepAlive final
    {
        static constexpr bool is_client = IsClient;
        static constexpr bool is_server = !IsClient;

        std::uint32_t id;

        /** @brief The number of seconds since the Unix epoch time. */
        std::uint32_t epoch;
    };
#pragma pack(pop)

    using ClientKeepAlive = KeepAlive<true>;
    using ServerKeepAlive = KeepAlive<false>;

    struct Segment final
    {
        using payload_type = std::variant<IPC, ClientKeepAlive, ServerKeepAlive>;

#pragma pack(push, 1)
        struct Header final
        {
            /** @brief The total length of the segment, including this header. */
            std::uint32_t size;

            /** @brief The actor ID that sent the segment. */
            std::uint32_t source;

            /** @brief The actor ID that receives the segment. */
            std::uint32_t target;

            /** @brief The type of segment. */
            SegmentType type;

            /** @brief Unknown value. Usually 0x0000. */
            std::uint16_t unknown;

            /** @return The length of the segment data. */
            inline decltype(size) data_size() const noexcept
            {
                return size - sizeof(*this);
            }
        };
#pragma pack(pop)

        /** @brief The segment header (metadata). */
        Header header;

        /** @brief The uncompressed segment payload. */
        std::vector<char> data;
    };

    void to_json(nlohmann::json& j, const Bundle& bundle);

    void to_json(nlohmann::json& j, const Segment& segment);
}  // namespace gunblade::ffxiv
