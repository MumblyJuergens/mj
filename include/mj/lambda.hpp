#pragma once

#include <functional>
#include <type_traits>
#include <utility>

// Courtesy of https://gist.github.com/isc30/fab67e5956fe8f2097bed84ebc42c1e8.

namespace mj::internal
{
    template <typename TCallable, typename TSignature>
    constexpr bool is_callable_as_v = std::is_constructible<std::function<TSignature>, TCallable>::value;

    template <typename TCallable, typename TSignature, typename = std::enable_if_t<is_callable_as_v<TCallable, TSignature>>>
    using Callable = TCallable;

    template <typename TCallable>
    struct CallableMetadata : CallableMetadata<decltype(&TCallable::operator())> {};

    template <class TClass, typename TReturn, typename... TArgs>
    struct CallableMetadata<TReturn (TClass::*)(TArgs...)>
    {
        using class_t = TClass;
        using return_t = TReturn;
        using args_tuple_t = std::tuple<TArgs...>;
        using ptr_t = TReturn (*)(TArgs...);

        // Beware! this function makes a copy of the closure! and of the arguments when called!
        static ptr_t generatePointer(const TClass &closure)
        {
            static TClass staticClosureCopy = closure;

            return [](TArgs... args)
            {
                return staticClosureCopy(args...);
            };
        }
    };

    template <class TClass, typename TReturn, typename... TArgs>
    struct CallableMetadata<TReturn (TClass::*)(TArgs...) const>
    {
        using class_t = TClass;
        using return_t = TReturn;
        using args_tuple_t = std::tuple<TArgs...>;
        using ptr_t = TReturn (*)(TArgs...);

        // Beware! this function makes a copy of the closure! and of the arguments when called!
        static ptr_t generatePointer(const TClass &closure)
        {
            static TClass staticClosureCopy = closure;

            return [](TArgs... args)
            {
                return staticClosureCopy(args...);
            };
        }
    };
} // End namespace mj::internal.

namespace mj
{
    // Beware! this function makes a copy of the closure! and of the arguments when called!
    template <typename TCallable>
    auto *capturing_lambda_pointer(const TCallable &callable)
    {
        return internal::CallableMetadata<TCallable>::generatePointer(callable);
    }

} // End namespace mj.