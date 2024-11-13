#pragma once

#include <concepts>

namespace mj
{
    template <typename T>
    concept hassize = requires(T t) {
        { t.size() } -> std::convertible_to<int>;
    };

    template <typename T>
        requires hassize<T>
    constexpr auto isize(const T &t)
    {
        return static_cast<int>(t.size());
    }

    template <typename T, std::size_t N>
    constexpr auto isize(const T (&)[N])
    {
        return static_cast<int>(N);
    }

} // End namespace mj.