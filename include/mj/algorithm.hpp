#pragma once

#include <functional>
#include <ranges>
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

    namespace pissoff {

        template<typename T>
        struct member_pointer_melper {};

        template<typename T, typename U>
        struct member_pointer_melper<T U:: *>
        {
            using type = T;
            using clazz = U;
        };

    } // End namespace pissoff.

    template<typename T>
    struct member_pointer_types : pissoff::member_pointer_melper<T> {};

    template<typename Func, typename Rhs, typename Comp>
    static constexpr auto magic_lambda(Func &&func, Comp compare, const Rhs value)
    {
        return [func, compare, value](member_pointer_types<Func>::clazz &n)
            {
                return std::invoke(compare, std::invoke(func, n), value);
            };
    }

    template<typename Rhs, typename Comp>
    static constexpr auto magic_lambda(Comp compare, const Rhs value)
    {
        return [compare, value](Rhs &n)
            {
                return std::invoke(compare, n, value);
            };
    }

    template<typename ...Args>
    constexpr auto filter(Args &&...args)
    {
        return std::views::filter(magic_lambda(std::forward<Args>(args)...));
    }

} // End namepspace mj.