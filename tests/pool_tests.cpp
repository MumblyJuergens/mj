#include "helpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <mj/pool.hpp>

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
