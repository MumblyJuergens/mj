#pragma once

#include <algorithm>

namespace mj
{
    // Non reference minmax.
    template <typename T>
    [[nodiscard]] constexpr std::pair<T, T> minmax(const T a, const T b) noexcept { return std::minmax(a, b); }

    template <typename T>
    [[nodiscard]] constexpr T difference(const T a, const T b) noexcept { return std::abs(a - b); }

    template <typename T>
    [[nodiscard]] constexpr auto count_digits(T t) { return static_cast<std::size_t>(std::floor(std::log10(t) + 1)); }

} // End namespace mj.