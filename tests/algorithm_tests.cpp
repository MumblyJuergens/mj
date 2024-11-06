#include <catch2/catch_test_macros.hpp>
#include <mj/algorithm.hpp>
#include <vector>

TEST_CASE("loop test with index", "[algorithm,loop]")
{
    std::size_t sum{};
    constexpr std::size_t expected = 0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
    
    mj::loop(10uz, [&sum](auto i){ sum += i; });

    REQUIRE(sum == expected);
}

TEST_CASE("loop test without index", "[algorithm,loop]")
{
    std::size_t sum{};
    constexpr std::size_t expected = 10;
    
    mj::loop(10, [&sum](){ ++sum; });

    REQUIRE(sum == expected);
}

TEST_CASE("sum object array by method call", "[algorithm,sum]")
{
    struct X
    {
        int a;
        int A() { return a; }
    };
    std::vector<X> vec{{0}, {3}, {1}, {5}};
    constexpr int expected = 0 + 3 + 1 + 5;

    const auto result = mj::sum(vec, &X::A);

    REQUIRE(result == expected);
}

TEST_CASE("sum object array by member access", "[algorithm,sum]")
{
    struct X
    {
        int a;
    };
    std::vector<X> vec{{0}, {3}, {1}, {5}};
    constexpr int expected = 0 + 3 + 1 + 5;

    const auto result = mj::sum(vec, &X::a);

    REQUIRE(result == expected);
}

