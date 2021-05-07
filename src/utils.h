#pragma once

#include <codecvt>  // codecvt_utf8
#include <locale>   // wstring_convert
#include <string>   // string

namespace gunblade
{
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
