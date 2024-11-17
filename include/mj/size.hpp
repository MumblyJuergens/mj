#pragma once

#include <concepts>
#include <gsl/narrow>

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
        return gsl::narrow<int>(t.size());
    }

    template <typename T, std::size_t N>
    constexpr auto isize(const T (&)[N]) noexcept
    {
        return gsl::narrow<int>(N);
    }

    template <typename T>
    constexpr auto sz_t(T t) noexcept
    {
        return gsl::narrow<std::size_t>(t);
    }

    template <typename T>
    constexpr int isizeof = gsl::narrow<int>(sizeof(T));

} // End namespace mj.