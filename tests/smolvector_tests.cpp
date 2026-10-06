#include "helpers.hpp"
#include <catch2/catch_test_macros.hpp>
#include <mj/smolvector.hpp>

TEST_CASE("construct and destruct", "[smolvector]") { mj::SmolVector<int> svint{}; }

TEST_CASE("empty test", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    REQUIRE(svint.size() == 0);
    REQUIRE(svint.capacity() == 0);
}

[[nodiscard]] constexpr static size_t next_pow2(size_t s) noexcept
{
    s |= s >> 1;
    s |= s >> 2;
    s |= s >> 4;
    s |= s >> 8;
    s |= s >> 16;
    return s + 1;
}

static_assert(next_pow2(0) == 1);
static_assert(next_pow2(1) == 2);
static_assert(next_pow2(2) == 4);
static_assert(next_pow2(3) == 4);
static_assert(next_pow2(4) == 8);
static_assert(next_pow2(5) == 8);
static_assert(next_pow2(6) == 8);
static_assert(next_pow2(7) == 8);
static_assert(next_pow2(8) == 16);
static_assert(next_pow2(9) == 16);
static_assert(next_pow2(10) == 16);

TEST_CASE("push item", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    svint.push_back(42);

    REQUIRE(svint.size() == 1);
    REQUIRE(svint.capacity() == 1);
}

TEST_CASE("push items", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    svint.push_back(42);
    REQUIRE(svint.size() == 1);
    REQUIRE(svint.capacity() == 1);
    REQUIRE(svint.at(0) == 42);

    svint.push_back(43);
    REQUIRE(svint.size() == 2);
    REQUIRE(svint.capacity() == 2);

    svint.push_back(44);
    REQUIRE(svint.size() == 3);
    REQUIRE(svint.capacity() == 4);

    svint.push_back(45);
    REQUIRE(svint.size() == 4);
    REQUIRE(svint.capacity() == 4);

    svint.push_back(46);
    REQUIRE(svint.size() == 5);
    REQUIRE(svint.capacity() == 8);
}

TEST_CASE("move contents if noexccept move", "[smolvector]")
{
    mj::SmolVector<CopyAndMoveNoexcept> svint_a{};
    mj::SmolVector<CopyAndMoveNoexcept> svint_b{};
    CopyAndMoveNoexcept src{3};

    svint_a.push_back(src);
    [[maybe_unused]] CopyAndMoveNoexcept *data = svint_a.data();

    REQUIRE(svint_a.size() == 1);
    REQUIRE(svint_a.capacity() == 1);
    REQUIRE(svint_a.at(0).moved == 13); // 3 at creation, 10 to copy into data

    svint_b = std::move(svint_a);

    REQUIRE(svint_a.size() == 0);
    REQUIRE(svint_a.capacity() == 0);
    REQUIRE(svint_b.size() == 1);
    REQUIRE(svint_b.capacity() == 1);
    REQUIRE(svint_b.at(0).moved == 13); // 13 before, 13 after move.
}

TEST_CASE("copy contents on copy", "[smolvector]")
{
    mj::SmolVector<CopyAndMove> svint_a{};
    mj::SmolVector<CopyAndMove> svint_b{};
    CopyAndMove src{3};

    svint_a.push_back(src);
    [[maybe_unused]] CopyAndMove *data = svint_a.data();

    REQUIRE(svint_a.size() == 1);
    REQUIRE(svint_a.capacity() == 1);
    REQUIRE(svint_a.at(0).moved == 13); // 3 at creation, 10 to copy into data

    svint_b = svint_a;

    REQUIRE(svint_a.size() == 1);
    REQUIRE(svint_a.capacity() == 1);
    REQUIRE(svint_b.size() == 1);
    REQUIRE(svint_b.capacity() == 1);
    REQUIRE(svint_b.at(0).moved == 23); // 13 before, 23 after copy.
}

TEST_CASE("Iterable", "[smolvector]")
{
    mj::SmolVector<int> svint{};
    svint.push_back(1);
    svint.push_back(5);
    svint.push_back(7);
    const int comparer[3]{1, 5, 7};
    size_t i{};

    for (auto n : svint) {
        REQUIRE(n == comparer[i++]);
    }
}

TEST_CASE("Const iterable", "[smolvector]")
{
    mj::SmolVector<int> svint{};
    svint.push_back(1);
    svint.push_back(5);
    svint.push_back(7);
    const int comparer[3]{1, 5, 7};
    size_t i{};

    for (auto it = svint.cbegin(); it != svint.cend(); ++it) {
        REQUIRE(*it == comparer[i++]);
    }
}

TEST_CASE("Reverse iterable", "[smolvector]")
{
    mj::SmolVector<int> svint{};
    svint.push_back(1);
    svint.push_back(5);
    svint.push_back(7);
    const int comparer[3]{7, 5, 1};
    size_t i{};

    for (auto it = svint.rbegin(); it != svint.rend(); ++it) {
        REQUIRE(*it == comparer[i++]);
    }
}

TEST_CASE("Const reverse iterable", "[smolvector]")
{
    mj::SmolVector<int> svint{};
    svint.push_back(1);
    svint.push_back(5);
    svint.push_back(7);
    const int comparer[3]{7, 5, 1};
    size_t i{};

    for (auto it = svint.crbegin(); it != svint.crend(); ++it) {
        REQUIRE(*it == comparer[i++]);
    }
}

TEST_CASE("Assing n values to empty", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    svint.assign(3, 42);

    REQUIRE(svint.capacity() == 4);
    REQUIRE(svint.size() == 3);
    REQUIRE(svint.at(0) == 42);
    REQUIRE(svint.at(1) == 42);
    REQUIRE(svint.at(2) == 42);
}

TEST_CASE("Assing n values to lesser", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    svint.assign(3, 42);
    svint.assign(5, 126);

    REQUIRE(svint.capacity() == 8);
    REQUIRE(svint.size() == 5);
    REQUIRE(svint.at(0) == 126);
    REQUIRE(svint.at(1) == 126);
    REQUIRE(svint.at(2) == 126);
    REQUIRE(svint.at(3) == 126);
    REQUIRE(svint.at(4) == 126);
}

TEST_CASE("Assing n values to greater", "[smolvector]")
{
    Counter::reset();
    CountingAllocator<Counter>::reset();
    mj::SmolVector<Counter, CountingAllocator<Counter>> sv{};

    sv.assign(5, 42);
    sv.assign(3, 126);

    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.size() == 3);
    REQUIRE(sv.at(0).value == 126);
    REQUIRE(sv.at(1).value == 126);
    REQUIRE(sv.at(2).value == 126);

    REQUIRE(Counter::value_constructed == 2); // One for each call to assign.
    REQUIRE(Counter::copy_constructed == 8);  // Each placed in fresh memory (rebuild because capacity changes).
    REQUIRE(Counter::copy_assigned == 0);     // Because we have a copy constructor.
    REQUIRE(Counter::move_constructed == 0);  // Shouldn't come up.
    REQUIRE(Counter::move_assigned == 0);     // Shouldn't come up.
    REQUIRE(Counter::destructored == 7);      // Original 5 + 2 from calls to assign.

    REQUIRE(CountingAllocator<Counter>::allocation_calls == 2);
    REQUIRE(CountingAllocator<Counter>::deallocation_calls == 1);
    REQUIRE(CountingAllocator<Counter>::allocations == 12);
    REQUIRE(CountingAllocator<Counter>::deallocations == 8);
}