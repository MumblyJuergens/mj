#pragma once

#include <algorithm>

namespace mj
{
    template<typename T>
    std::pair<T,T> minmax(const T a, const T b) noexcept { return std::minmax(a, b); }

} // End namespace mj.