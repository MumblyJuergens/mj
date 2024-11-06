#pragma once

#include <limits>
#include <utility>
#include <vector>

namespace mj
{
    enum class PoolToyOrigins
    {
        fresh,
        used,
        error,
    };

    template <typename T>
    class [[nodiscard]] Pool final
    {
        std::vector<T> data;
        std::vector<std::size_t> free;

        public:

        class [[nodiscard]] Toy final
        {
            friend Pool;
            static constexpr std::size_t bad = std::numeric_limits<std::size_t>::max();

            Pool *pool{};
            std::size_t index{bad};

            [[nodiscard]] constexpr Toy(Pool *pool, std::size_t index) noexcept : pool{pool}, index{index} {}

        public:
            [[nodiscard]] constexpr Toy() noexcept = default;
            [[nodiscard]] constexpr Toy(Toy &&other) noexcept : pool{std::exchange(other.pool, nullptr)}, index{std::exchange(other.index, bad)} {}
            constexpr Toy &operator=(Toy &&other) noexcept
            {
                pool = std::exchange(other.pool, nullptr);
                if (index != bad && pool)
                {
                    pool->free.push_back(index);
                }
                index = std::exchange(other.index, bad);
                return *this;
            }
            constexpr ~Toy() noexcept(noexcept(std::vector<std::size_t>{}.push_back(std::size_t{})))
            {
                if (index != bad && pool)
                {
                    pool->free.push_back(index);
                }
            }
            [[nodiscard]] constexpr T *operator->() const { return &pool->data[index]; }
            [[nodiscard]] constexpr T &operator*() const { return pool->data[index]; }
            [[nodiscard]] constexpr operator bool() const noexcept { return index != bad; }
        };

        template <typename... Args>
        [[nodiscard]] constexpr std::pair<Toy, PoolToyOrigins> get(Args &&...args)
        {
            if (!free.empty())
            {
                Toy toy{this, free.back()};
                free.pop_back();
                return {std::move(toy), PoolToyOrigins::used};
            }
            data.emplace_back(args...);
            return {Toy{this, data.size() - 1}, PoolToyOrigins::fresh};
        }
    };

} // End of namespace mj.