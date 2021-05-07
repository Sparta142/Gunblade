#include "tcp_table.h"

#include <array>
#include <cstddef>
#include <optional>

#include <Windows.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iphlpapi.h>

template <std::size_t AllocThreshold>
class ScratchBuffer  // TODO: Remove this and use a std::vector<std::byte>
{
public:
    using backing_type = std::byte;

    explicit ScratchBuffer() = default;

    inline void* get() noexcept
    {
        return heap_buf_ ? heap_buf_->data() : stack_buf_.data();
    }

    inline std::size_t size() const noexcept
    {
        return heap_buf_ ? heap_buf_->size() : stack_buf_.size();
    }

    void resize(std::size_t size)
    {
        if (!heap_buf_)
        {
            if (size <= AllocThreshold)
            {
                // It's small enough to stay on the stack, which
                // means we already have that much space allocated.
                return;
            }

            // Too big to leave on the stack, copy the existing
            // data to a vector before reserving the space.
            heap_buf_ = std::vector<backing_type>(stack_buf_.cbegin(), stack_buf_.cend());
        }

        heap_buf_->reserve(size);
    }

private:
    std::size_t size_ = AllocThreshold;

    std::array<backing_type, AllocThreshold> stack_buf_;
    std::optional<std::vector<backing_type>> heap_buf_ = std::nullopt;
};

// TODO: Make iterable TcpTableV4/V6 classes that store an internal buffer

static DWORD GetExtendedTcpTable(PVOID pTcpTable, PDWORD pdwSize, ULONG ulAf)
{
    return GetExtendedTcpTable(
        pTcpTable, pdwSize, false, ulAf, TCP_TABLE_CLASS::TCP_TABLE_OWNER_PID_CONNECTIONS, 0);
}

namespace gunblade
{
    static ConnectionInfo info_from_tcprow(const MIB_TCPROW_OWNER_PID& row)
    {
        return ConnectionInfo{
            Tins::IPv4Address(row.dwLocalAddr),
            ntohs(static_cast<unsigned short>(row.dwLocalPort)),
            Tins::IPv4Address(row.dwRemoteAddr),
            ntohs(static_cast<unsigned short>(row.dwRemotePort)),
            static_cast<TcpState>(row.dwState),
            row.dwOwningPid};
    }

    static ConnectionInfo info_from_tcp6row(const MIB_TCP6ROW_OWNER_PID& row)
    {
        return ConnectionInfo{
            Tins::IPv6Address(row.ucLocalAddr),
            ntohs(static_cast<unsigned short>(row.dwLocalPort)),
            Tins::IPv6Address(row.ucRemoteAddr),
            ntohs(static_cast<unsigned short>(row.dwRemotePort)),
            static_cast<TcpState>(row.dwState),
            row.dwOwningPid};
    }

    template <typename Table, typename Converter>
    static std::vector<ConnectionInfo> get_table_impl(ULONG ulAf, Converter converter)
    {
        ScratchBuffer<4096> buf;
        DWORD size = static_cast<DWORD>(buf.size());
        DWORD error;

        do
        {
            buf.resize(size);
            error = GetExtendedTcpTable(buf.get(), &size, ulAf);
        } while (error == ERROR_INSUFFICIENT_BUFFER);

        if (error != ERROR_SUCCESS)
        {
            throw std::exception("GetExtendedTcpTable failed");
        }

        const auto* table = static_cast<Table*>(buf.get());

        std::vector<ConnectionInfo> rows;
        rows.reserve(table->dwNumEntries);

        for (decltype(table->dwNumEntries) i = 0; i < table->dwNumEntries; ++i)
        {
            rows.emplace_back(std::invoke(converter, table->table[i]));
        }

        return rows;
    }

    std::vector<ConnectionInfo> get_tcp_table()
    {
        auto ipv4_table = get_table_impl<MIB_TCPTABLE_OWNER_PID>(AF_INET, &info_from_tcprow);
        auto ipv6_table = get_table_impl<MIB_TCP6TABLE_OWNER_PID>(AF_INET6, &info_from_tcp6row);
        ipv4_table.insert(ipv4_table.end(), ipv6_table.begin(), ipv6_table.end());

        return ipv4_table;
    }
}  // namespace gunblade
