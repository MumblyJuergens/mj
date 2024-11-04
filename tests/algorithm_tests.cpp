#include <catch2/catch_test_macros.hpp>
#include <mj/algorithm.hpp>

TEST_CASE("loop test with index", "[algorithm,loop]")
{
    std::size_t sum{};
    mj::loop(10uz, [&sum](auto i){ sum += i; });
    constexpr std::size_t expected = 0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;

    REQUIRE(sum == expected);
}

TEST_CASE("loop test without index", "[algorithm,loop]")
{
    std::size_t sum{};
    mj::loop(10, [&sum](){ ++sum; });
    constexpr std::size_t expected = 10;

    REQUIRE(sum == expected);
}