#include <utility>
#include <catch2/catch_test_macros.hpp>
#include <mj/pool.hpp>

struct [[nodiscard]] MoveOnly
{
    int moved{};

    [[nodiscard]] MoveOnly() = default;
    [[nodiscard]] MoveOnly(MoveOnly &&o) noexcept : moved{std::exchange(o.moved, 0) + 1} {}
    MoveOnly &operator=(MoveOnly &&o) noexcept
    {
        moved = std::exchange(o.moved, 0) + 1;
        return *this;
    }
    MoveOnly(const MoveOnly &) = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;
};

TEST_CASE("Pool return value optimisation", "[pool]")
{
    mj::Pool<MoveOnly> pool;

    auto [mo, status] = pool.get();

    REQUIRE(status == mj::PoolToyOrigins::fresh);
    REQUIRE(mo->moved == 0);
}

TEST_CASE("Pool moves", "[pool]")
{
    mj::Pool<MoveOnly> pool;
    mj::PoolToyOrigins originalReturnType = mj::PoolToyOrigins::error;
    {
        auto [mo, status] = pool.get();
        originalReturnType = status;

        // Returned to pool by dtor here...
    }
    auto [mo, status] = pool.get();

    REQUIRE(originalReturnType == mj::PoolToyOrigins::fresh);
    REQUIRE(status == mj::PoolToyOrigins::used);
    REQUIRE(mo->moved == 0);
}
