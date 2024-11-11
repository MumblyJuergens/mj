#pragma once

#include <iterator>
#include <cstddef>

namespace mj
{

    struct [[nodiscard]] InsertCounter final
    {
        std::size_t count{};
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = void;
        using container_type = void;
        constexpr InsertCounter &operator=(auto &&) noexcept
        {
            ++count;
            return *this;
        }
        constexpr InsertCounter &operator*() noexcept { return *this; }
        constexpr InsertCounter &operator++() noexcept { return *this; }
        constexpr InsertCounter &operator++(int) noexcept { return *this; }
    };

} // End namsepace mj.