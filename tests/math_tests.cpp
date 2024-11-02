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