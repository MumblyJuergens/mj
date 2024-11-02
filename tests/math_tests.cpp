#include <catch2/catch_test_macros.hpp>
#include <mj/math.hpp>

TEST_CASE("minmax easy test", "[math]")
{
    int a = 10;
    int b = 20;
    auto [min, max] = mj::minmax(a, b);
    REQUIRE(min == a);
    REQUIRE(max == b);
}

TEST_CASE("difference basic test", "[math]")
{
    REQUIRE(mj::difference(10, 20) == 10);
    REQUIRE(mj::difference(-10, 20) == 30);
    REQUIRE(mj::difference(20, -10) == 30);
}