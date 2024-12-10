#pragma once

#include <string_view>

#define nameof(x) mj::internal::_nameof(#x, sizeof(x))
#define nameofc(x) mj::internal::_nameof(#x, sizeof(x)).data()
#define nameofanything(x) mj::internal::_nameof(#x, 0)

namespace mj::internal
{
    consteval std::string_view _nameof(std::string_view x, std::size_t)
    {
        auto lastPointerDefreference = x.rfind("->");
        if (lastPointerDefreference != std::string_view::npos)
        {
            x = std::string_view(x.data() + lastPointerDefreference + 2);
        }
        if (auto m = x.find_last_of(".:"))
        {
            if (m != std::string_view::npos)
            {
                return x.data() + m + 1;
            }
        }
        return x;
    }

} // End namespace mj::internal.