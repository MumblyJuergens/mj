#pragma once

#include <algorithm>

namespace mj
{
    // Non reference minmax.
    template<typename T>
    std::pair<T,T> minmax(const T a, const T b) noexcept { return std::minmax(a, b); }

    template<typename T>
    T difference(const T a, const T b) noexcept { return std::abs(a - b);}

} // End namespace mj.