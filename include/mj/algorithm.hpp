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

    template <typename Range, typename Proj>
    using sum_result_t = std::decay_t<std::invoke_result_t<Proj &, std::ranges::range_value_t<Range>>>;

    template <std::ranges::input_range Range, typename Proj = std::identity>
    constexpr auto sum(Range &&range, Proj proj)
    {
        sum_result_t<Range, Proj> r{};
        for (auto i = begin(range); i != end(range); ++i)
        {
            r += std::invoke(proj, *i);
        }
        return r;
    }

    template <typename R, typename T>
    R force_explicit_cast(T x) { return static_cast<R>(x); }

    template <std::ranges::input_range Range, typename Proj = std::identity>
    auto mean(Range &&range, Proj proj = {})
    {
        return sum(range, proj) / force_explicit_cast<float>(size(range));
    }

    template <typename T>
    auto square(T &&x) { return x * x; };

    template <std::ranges::input_range Range, typename Mean = std::ranges::range_value_t<Range>, typename Proj = std::identity>
    auto stddev(Range &&range, Mean mean, Proj proj = {})
    {
        return sqrt(mj::sum(range, [&](const auto &x) -> Mean
                            { return square(std::invoke(proj, x) - mean); }) /
                    force_explicit_cast<float>(size(range)));
    }

    template <typename Range, typename Proj = std::identity>
    auto mean_stddev(Range &&range, Proj &&proj = {})
    {
        auto mea = mean(range, proj);
        return std::make_pair(mea, stddev(range, mea, proj));
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

    template <typename Range, typename Func>
    constexpr auto for_each(Range &&range, Func func)
    {
        for (std::size_t i{}; i < size(range); ++i)
        {
            func(range.at(i), i);
        }
    }

} // End namepspace mj.