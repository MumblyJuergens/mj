#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <mj/size.hpp>

TEST_CASE("isize from container size", "[isize]")
{
    const std::vector<int> vec{1, 3, 5, 7};

    auto result = mj::isize(vec);

    static_assert(std::is_same_v<decltype(result), int>);
    REQUIRE(result == 4);
}

TEST_CASE("isize from array size", "[isize]")
{
    const int arr[]{1, 3, 5, 7, 9};

    auto result = mj::isize(arr);

    static_assert(std::is_same_v<decltype(result), int>);
    REQUIRE(result == 5);
}

TEST_CASE("size_t from int", "[sz_t]")
{
    int i = 5;

    auto s = mj::sz_t(i);

    REQUIRE(std::is_same_v<decltype(s), std::size_t>);
}

TEST_CASE("int from isizeof", "[isizeof]")
{
    struct alignas(4) S
    {
        std::uint32_t b;
        std::uint32_t a;
    };

    constexpr auto s = mj::isizeof<S>;

    REQUIRE(s == 8);
}