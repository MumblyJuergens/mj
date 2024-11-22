#pragma once

#include <print>

namespace mj
{

    inline void clear_console() { std::print("\x1B[3J\x1B[H"); }
    inline void clear_console_properly() { std::print("\x1B[2J\x1B[H"); }

} // End namespace mj.