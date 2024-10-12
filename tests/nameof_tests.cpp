#include <catch2/catch_test_macros.hpp>
#include <mj/nameof.hpp>

TEST_CASE("nameof with struct type", "[nameof]")
{
    struct a
    {
        int b;
    };
    constexpr auto value = nameof(a::b);
    REQUIRE(std::string{value} == "b");
}

TEST_CASE("nameof with fundamental object", "[nameof]")
{
    [[maybe_unused]] int b;
    constexpr auto value = nameof(b);
    REQUIRE(std::string{value} == "b");
}

TEST_CASE("nameof with fundamental type", "[nameof]")
{
    constexpr auto value = nameof(int);
    REQUIRE(std::string{value} == "int");
}

TEST_CASE("nameof with member dereference", "[nameof]")
{
    struct a
    {
        int b;
    };
    [[maybe_unused]] a c;

    constexpr auto value = nameof(c.b);
    REQUIRE(std::string{value} == "b");
}

TEST_CASE("nameof with pointer dereference", "[nameof]")
{
    struct a
    {
        int b;
    };
    [[maybe_unused]] a e;
    [[maybe_unused]] a *c = &e;

    constexpr auto value = nameof(c->b);
    REQUIRE(std::string{value} == "b");
}

TEST_CASE("nameofanything with namespace", "[nameof]")
{
    constexpr auto value = nameofanything(glube);
    REQUIRE(std::string{value} == "glube");
}

TEST_CASE("nameof with member function", "[nameof]")
{
    struct a
    {
        int b();
    };
    [[maybe_unused]] a c;

    constexpr auto value = nameof(&a::b);
    REQUIRE(std::string{value} == "b");
}