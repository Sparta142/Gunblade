#pragma once

#include "structs.h"

#include <deque>
#include <limits>
#include <optional>
#include <vector>

namespace machinist
{
    using namespace machinist::ffxiv;

    class FinalFantasyDecoder
    {
        using storage_type = uint8_t;

    public:
        template <typename InputIterator>
        void feed_data(InputIterator first, InputIterator last)
        {
            data_.insert(data_.end(), first, last);
        }

        size_t size() const noexcept
        {
            return data_.size();
        }

        void clear() noexcept
        {
            data_.clear();
        }

        std::optional<Bundle> next_bundle();

    private:
        std::deque<storage_type> data_;
    };
}  // namespace machinist
