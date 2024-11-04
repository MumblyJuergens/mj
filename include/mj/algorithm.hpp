#pragma once

#include <type_traits>
#include <cstddef>

namespace mj
{
    template<typename Func, typename Index = std::size_t>
    constexpr void loop(const Index n, const Func &func)
    {
        for (auto i = Index{}; i < n; ++i)
        {
            if constexpr (std::is_invocable_v<Func, std::size_t>)
            {
                func(i);
            }
            else if constexpr (std::is_invocable_v<Func>)
            {
                func();
            }
        }
    }
}