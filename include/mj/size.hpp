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
    constexpr auto isize(const T &t) noexcept
    {
        return static_cast<int>(t.size());
    }

    template <typename T, std::size_t N>
    constexpr auto isize(const T (&)[N]) noexcept
    {
        return static_cast<int>(N);
    }

    template <typename T>
    constexpr auto sz_t(T t) noexcept
    {
        return static_cast<std::size_t>(t);
    }

} // End namespace mj.