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

        Pool *_pool{};
        std::size_t _index{bad};

        [[nodiscard]] constexpr Toy(Pool *pool, std::size_t index) noexcept : _pool{pool}, _index{index} {}

      public:
        [[nodiscard]] constexpr Toy() noexcept = default;
        [[nodiscard]] constexpr Toy(Toy &&other) noexcept
            : _pool{std::exchange(other._pool, nullptr)}, _index{std::exchange(other._index, bad)}
        {
        }
        constexpr Toy &operator=(Toy &&other) noexcept
        {
            _pool = std::exchange(other._pool, nullptr);
            if (_index != bad && _pool) {
                _pool->free.push_back(_index);
            }
            _index = std::exchange(other._index, bad);
            return *this;
        }
        constexpr ~Toy() noexcept(noexcept(std::vector<std::size_t>{}.push_back(std::size_t{})))
        {
            if (_index != bad && _pool) {
                _pool->free.push_back(_index);
            }
        }
        [[nodiscard]] constexpr T *operator->() const { return &_pool->data[_index]; }
        [[nodiscard]] constexpr T &operator*() const { return _pool->data[_index]; }
        [[nodiscard]] constexpr operator bool() const noexcept { return _index != bad; }
    };

    template <typename... Args>
    [[nodiscard]] constexpr std::pair<Toy, PoolToyOrigins> get(Args &&...args)
    {
        if (!free.empty()) {
            Toy toy{this, free.back()};
            free.pop_back();
            return {std::move(toy), PoolToyOrigins::used};
        }
        data.emplace_back(args...);
        return {Toy{this, data.size() - 1}, PoolToyOrigins::fresh};
    }
};

} // End of namespace mj.