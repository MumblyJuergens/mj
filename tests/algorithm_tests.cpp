#include <catch2/catch_test_macros.hpp>
#include <mj/algorithm.hpp>
#include <ranges>
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

TEST_CASE("magic_lambda member less test", "[algorithm,magic_lambda]")
{
    struct X
    {
        int a;
    };
    std::vector<X> vec{{0}, {3}, {1}, {5}};
    constexpr int expected = 0 + 3 + 1;
    int result{};

    for (const X &x : vec | std::views::filter(mj::magic_lambda(&X::a, std::less{}, 5)))
    {
        result += x.a;
    }

    REQUIRE(result == expected);
}

TEST_CASE("filter member less test", "[algorithm,filter,magic_lambda]")
{
    struct X
    {
        int a;
    };
    std::vector<X> vec{{0}, {3}, {1}, {5}};
    constexpr int expected = 0 + 3 + 1;
    int result{};

    for (const X &x : vec | mj::filter(&X::a, std::less{}, 5))
    {
        result += x.a;
    }

    REQUIRE(result == expected);
}

TEST_CASE("filter method less test", "[algorithm,filter,magic_lambda]")
{
    struct X
    {
        int a;
        int A() { return a; }
    };
    std::vector<X> vec{{0}, {3}, {1}, {5}};
    constexpr int expected = 0 + 3 + 1;
    int result{};

    for (const X &x : vec | mj::filter(&X::A, std::less{}, 5))
    {
        result += x.a;
    }

    REQUIRE(result == expected);
}

TEST_CASE("filter value less test", "[algorithm,filter,magic_lambda]")
{
    std::vector<int> vec{0, 3, 1, 5};
    constexpr int expected = 0 + 3 + 1;
    int result{};

    for (const int &x : vec | mj::filter(std::less{}, 5))
    {
        result += x;
    }

    REQUIRE(result == expected);
}

