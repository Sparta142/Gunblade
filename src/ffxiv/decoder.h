#pragma once

#include "structs.h"

#include <deque>
#include <optional>

namespace gunblade
{
    using namespace gunblade::ffxiv;

    class FinalFantasyDecoder
    {
        using storage_type = uint8_t;

    public:
        template <typename InputIterator>
        inline void feed_data(InputIterator first, InputIterator last)
        {
            data_.insert(data_.end(), first, last);
        }

        inline size_t size() const noexcept
        {
            return data_.size();
        }

        inline void clear() noexcept
        {
            data_.clear();
        }

        std::optional<Bundle> next_bundle();

    private:
        std::deque<storage_type> data_;
    };
}  // namespace gunblade
