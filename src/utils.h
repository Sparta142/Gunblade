#pragma once

#include <codecvt>
#include <cstddef>
#include <locale>
#include <string>
#include <utility>

namespace gunblade
{
    inline void hash_combine(std::size_t&) {}

    // Credit: https://stackoverflow.com/a/38140932/2958344
    template <typename T, typename... Args>
    inline void hash_combine(std::size_t& seed, const T& first, const Args&... args) noexcept
    {
        std::hash<T> hasher;
        seed ^= hasher(first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hash_combine(seed, args...);
    }

    template <typename String>
    inline std::string to_utf8(const String& wstr)
    {
        using value_type = String::value_type;
        using codecvt_type = std::codecvt_utf8<value_type>;
        using convert_type = std::wstring_convert<codecvt_type, value_type>;

        convert_type converter;
        return converter.to_bytes(wstr.data());
    }

    /**
     * @brief Get the name of the process identified by @p pid.
     *
     * @param pid The process ID to look up
     * @return `std::string` The name of the process, or
     * an empty string if the name is unable to be retrieved.
     */
    std::string get_process_name(unsigned long pid);
}  // namespace gunblade
