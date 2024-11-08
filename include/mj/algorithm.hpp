#pragma once

#include <functional>
#include <ranges>
#include <type_traits>
#include <cstddef>

namespace mj
{
    template <typename Func, typename Index = std::size_t>
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

    template <typename Range, typename Func>
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

    // Courtesy of https://stackoverflow.com/a/15218327/26759349.
    template <typename T>
    class member_projection
    {
        template <typename U>
        struct Tester
        {
            static_assert( // will always fail
                std::is_member_function_pointer<U>::value,
                "Use member function pointers only!");

            // if you want to report false for types other than
            // member function pointers you can just derive from
            // std::false_type instead of asserting
        };

        template <typename R, typename U>
        struct Tester<R U::*>
        {
            using clazz = U;
        };

        template <typename R, typename U, typename... Args>
        struct Tester<R (U::*)(Args...)> : std::false_type
        {
            using clazz = U;
        };

        template <typename R, typename U, typename... Args>
        struct Tester<R (U::*)(Args...) const> : std::true_type
        {
            using clazz = const U;
        };

        template <typename R, typename U, typename... Args>
        struct Tester<R (U::*)(Args...) noexcept> : std::false_type
        {
            using clazz = U;
        };

        template <typename R, typename U, typename... Args>
        struct Tester<R (U::*)(Args...) const noexcept> : std::true_type
        {
            using clazz = const U;
        };

    public:
        static constexpr bool is_const_mem_fn = Tester<typename std::remove_cv<T>::type>::value;
        using clazz = Tester<T>::clazz;
    };

    template <typename Func, typename Rhs, typename Comp>
    static constexpr auto magic_lambda(Func &&func, Comp compare, const Rhs value)
    {
        return [func, compare, value](member_projection<Func>::clazz &n)
        {
            return std::invoke(compare, std::invoke(func, n), value);
        };
    }

    template <typename Rhs, typename Comp>
    static constexpr auto magic_lambda(Comp compare, const Rhs value)
    {
        return [compare, value](Rhs &n)
        {
            return std::invoke(compare, n, value);
        };
    }

    template <typename Func, typename Rhs, typename Comp>
    struct magic_callable
    {
        const Func &func;
        const Comp &compare;
        const Rhs value;

        constexpr magic_callable(const Func &func, const Comp &compare, const Rhs value)
            : func{func}, compare{compare}, value{value}
        {
        }

        constexpr auto operator()(member_projection<Func>::clazz &n)
        {
            return std::invoke(compare, std::invoke(func, n), value);
        };

        // constexpr auto operator()(const member_projection<Func>::clazz &n)
        // {
        //     return std::invoke(compare, std::invoke(func, n), value);
        // };
    };

    template <typename... Args>
    constexpr auto filter(Args &&...args)
    {
        return std::views::filter(magic_lambda(std::forward<Args>(args)...));
    }

} // End namepspace mj.