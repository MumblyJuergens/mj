#pragma once

#include <limits>
#include <utility>
#include <vector>

namespace mj
{
    enum class PoolToyOrigins
    {
        New,
        Used,
        Error,
    };

    template <typename T>
    class Pool
    {
        std::vector<T> data;
        std::vector<std::size_t> free;

        public:

        class Toy
        {
            friend Pool;
            static constexpr std::size_t bad = std::numeric_limits<std::size_t>::max();

            Pool *pool{};
            std::size_t index{bad};

            Toy(Pool *pool, std::size_t index) : pool{pool}, index{index} {}

        public:
            Toy() = default;
            Toy(Toy &&other) noexcept : pool{std::exchange(other.pool, nullptr)}, index{std::exchange(other.index, bad)} {}
            Toy &operator=(Toy &&other) noexcept
            {
                pool = std::exchange(other.pool, nullptr);
                if (index != bad && pool)
                {
                    pool->free.push_back(index);
                }
                index = std::exchange(other.index, bad);
                return *this;
            }
            ~Toy()
            {
                if (index != bad && pool)
                {
                    pool->free.push_back(index);
                }
            }
            T *operator->() const { return &pool->data[index]; }
            T &operator*() const { return pool->data[index]; }
            operator bool() const noexcept { return index != bad; }
        };

        template <typename... Args>
        [[nodiscard]] std::pair<Toy, PoolToyOrigins> get(Args &&...args)
        {
            if (!free.empty())
            {
                Toy toy{this, free.back()};
                free.pop_back();
                return {std::move(toy), PoolToyOrigins::Used};
            }
            data.emplace_back(args...);
            return {Toy{this, data.size() - 1}, PoolToyOrigins::New};
        }
    };

} // End of namespace mj.