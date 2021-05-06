#include "utils.h"

#include <array>

#include <Windows.h>
#include <Psapi.h>

namespace gunblade
{
    std::string get_process_name(unsigned long pid)
    {
        auto hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, pid);

        if (hProcess == INVALID_HANDLE_VALUE)
        {
            return std::string();
        }

        std::array<char, FILENAME_MAX> buf;
        auto length =
            GetModuleBaseName(hProcess, nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        CloseHandle(hProcess);

        return std::string(buf.data(), length);
    }
}  // namespace gunblade
