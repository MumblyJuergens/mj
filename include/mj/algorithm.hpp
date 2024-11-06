#pragma once

#include <type_traits>
#include <iterator>
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

    template<typename Range, typename Func>
    constexpr auto sum(Range &&range, Func &&f)
    {
        using result_t = std::remove_cvref_t<std::invoke_result_t<Func, decltype(*begin(range))>>;
        result_t r{};
        for (auto i = begin(range); i != end(range); ++i)
        {
            r += std::invoke(f, *i);
        }
        return r;
    }
}