#pragma once

#include <chrono>
#include <concepts>

namespace mj
{

    template <std::floating_point T = float>
    class [[nodiscard]] Timer final
    {

        std::chrono::high_resolution_clock::time_point m_time_last{std::chrono::high_resolution_clock::now()};
        std::chrono::duration<T> m_time_elapsed{};

    public:
        void tick() noexcept
        {
            std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();
            m_time_elapsed = timeNow - m_time_last;
            m_time_last = timeNow;
        }

        [[nodiscard]] T elapsed() const noexcept
        {
            return m_time_elapsed.count();
        }

        template <typename D>
        [[nodiscard]] auto elapsed() const noexcept
        {
            return std::chrono::duration_cast<D>(m_time_elapsed);
        }
    };

} // End of namespace mj.