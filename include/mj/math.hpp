#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>

namespace mj
{
// Non reference minmax.
template <typename T>
[[nodiscard]] constexpr std::pair<T, T> minmax(const T a, const T b) noexcept
{
    return std::minmax(a, b);
}

template <typename T>
[[nodiscard]] constexpr T difference(const T a, const T b) noexcept
{
    return std::abs(a - b);
}

template <typename T>
[[nodiscard]] constexpr auto count_digits(T t)
{
    return static_cast<std::size_t>(std::floor(std::log10(t) + 1));
}

template <std::unsigned_integral T>
[[nodiscard]] constexpr static T next_pow2(T s) noexcept
{
    s |= s >> 1;
    s |= s >> 2;
    s |= s >> 4;
    s |= s >> 8;
    s |= s >> 16;
    return s + 1;
}

} // End namespace mj.
