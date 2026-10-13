#include "helpers.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <mj/smolvector.hpp>
#include <ranges>

TEST_CASE("Construct and destruct", "[smolvector]")
{
    mj::SmolVector<int> svint{};
    REQUIRE(svint.empty() == true);
}

TEST_CASE("Construct with allocator", "[smolvector]")
{
    std::allocator<int> allocator;
    mj::SmolVector<int> svint{allocator};
}

TEST_CASE("Count default construction", "[smolvector]")
{
    mj::SmolVector<int> sv(5uz);

    REQUIRE(sv.size() == 5);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv.empty() == false);
    REQUIRE(sv.at(0) == 0);
    REQUIRE(sv.at(1) == 0);
    REQUIRE(sv.at(2) == 0);
    REQUIRE(sv.at(3) == 0);
    REQUIRE(sv.at(4) == 0);
}

TEST_CASE("Count value construction", "[smolvector]")
{
    mj::SmolVector<int> sv(4uz, 8);

    REQUIRE(sv.size() == 4);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.at(0) == 8);
    REQUIRE(sv.at(1) == 8);
    REQUIRE(sv.at(2) == 8);
    REQUIRE(sv.at(3) == 8);
}

TEST_CASE("Iterator construction", "[smolvector]")
{
    std::array<int, 3> input{9, 7, 6};
    mj::SmolVector<int> sv{input.begin(), input.end()};

    REQUIRE(sv.size() == 3);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.at(0) == 9);
    REQUIRE(sv.at(1) == 7);
    REQUIRE(sv.at(2) == 6);
}

TEST_CASE("Range construction", "[smolvector]")
{
    std::array<int, 3> input{9, 7, 6};
    mj::SmolVector<int> sv{std::from_range, input};

    REQUIRE(sv.size() == 3);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.at(0) == 9);
    REQUIRE(sv.at(1) == 7);
    REQUIRE(sv.at(2) == 6);
}

TEST_CASE("Initializer list construction", "[smolvector]")
{
    mj::SmolVector<int> sv{{9, 7, 6}};

    REQUIRE(sv.size() == 3);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.at(0) == 9);
    REQUIRE(sv.at(1) == 7);
    REQUIRE(sv.at(2) == 6);
}

TEST_CASE("Initializer list assignment op", "[smolvector]")
{
    mj::SmolVector<int> sv{};
    sv = {9, 7, 6};

    REQUIRE(sv.size() == 3);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.at(0) == 9);
    REQUIRE(sv.at(1) == 7);
    REQUIRE(sv.at(2) == 6);
}

TEST_CASE("Empty test", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    REQUIRE(svint.size() == 0);
    REQUIRE(svint.capacity() == 0);
}

TEST_CASE("Push item", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    svint.push_back(42);

    REQUIRE(svint.size() == 1);
    REQUIRE(svint.capacity() == 1);
}

TEST_CASE("Push items", "[smolvector]")
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

TEST_CASE("Move contents if noexccept move", "[smolvector]")
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

TEST_CASE("Copy contents on copy", "[smolvector]")
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

TEST_CASE("Assigning n values to empty", "[smolvector]")
{
    mj::SmolVector<int> svint{};

    svint.assign(3, 42);

    REQUIRE(svint.capacity() == 4);
    REQUIRE(svint.size() == 3);
    REQUIRE(svint.at(0) == 42);
    REQUIRE(svint.at(1) == 42);
    REQUIRE(svint.at(2) == 42);
}

TEST_CASE("Assigning n values to lesser", "[smolvector]")
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

TEST_CASE("Assigning n values to greater", "[smolvector]")
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

TEST_CASE("Assigning n values to greater 32", "[smolvector]")
{
    Counter::reset();
    CountingAllocator<Counter>::reset();
    mj::SmolVector<Counter, CountingAllocator<Counter>, mj::impl::Smol32<Counter>> sv{};

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

TEST_CASE("Iterator assignment", "[smolvector]")
{
    mj::SmolVector<int> sv{};
    std::array<int, 5> input{2, 5, 7, 9, 1};

    sv.assign(input.begin(), input.end());

    REQUIRE(sv.size() == 5);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv.at(0) == 2);
    REQUIRE(sv.at(1) == 5);
    REQUIRE(sv.at(2) == 7);
    REQUIRE(sv.at(3) == 9);
    REQUIRE(sv.at(4) == 1);
}

TEST_CASE("Range assignment", "[smolvector]")
{
    mj::SmolVector<int> sv{};
    std::array<int, 5> input{2, 5, 7, 9, 1};

    sv.assign_range(input);

    REQUIRE(sv.size() == 5);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv.at(0) == 2);
    REQUIRE(sv.at(1) == 5);
    REQUIRE(sv.at(2) == 7);
    REQUIRE(sv.at(3) == 9);
    REQUIRE(sv.at(4) == 1);
}

TEST_CASE("Initializer list assignment", "[smolvector]")
{
    mj::SmolVector<int> sv{};

    sv.assign({1, 9, 7});

    REQUIRE(sv.size() == 3);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv.at(0) == 1);
    REQUIRE(sv.at(1) == 9);
    REQUIRE(sv.at(2) == 7);
}

TEST_CASE("Front and back work", "[smolvector]")
{
    mj::SmolVector<int> sv{1, 9, 5, 6, 345};

    REQUIRE(sv.size() == 5);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv.front() == 1);
    REQUIRE(sv.back() == 345);
}

TEST_CASE("Clear works", "[smolvector]")
{
    mj::SmolVector<int> sv{1, 9, 5, 6, 345};
    REQUIRE(sv.size() == 5);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv.empty() == false);

    sv.clear();
    REQUIRE(sv.size() == 0);
    REQUIRE(sv.capacity() == 0);
    REQUIRE(sv.empty() == true);
}

TEST_CASE("Inserting rvalue", "[smolvector]")
{
    mj::SmolVector<int> sv{12, 45, 98};

    sv.insert(sv.cbegin(), 87);

    REQUIRE(sv.size() == 4);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv[0] == 87);
    REQUIRE(sv[1] == 12);
    REQUIRE(sv[2] == 45);
    REQUIRE(sv[3] == 98);
}

TEST_CASE("Inserting lvalue", "[smolvector]")
{
    mj::SmolVector<Counter> sv{};
    Counter::reset();
    Counter counter{10};

    sv.insert(sv.cbegin(), std::move(counter));

    REQUIRE(sv.size() == 1);
    REQUIRE(sv.capacity() == 1);
    REQUIRE(sv[0].value == 10);
    REQUIRE(counter.value == 0);
    REQUIRE(Counter::move_constructed == 1);
}

TEST_CASE("Inserting rvalue with amount 1 at begin", "[smolvector]")
{
    mj::SmolVector<int> sv{12, 45, 98};

    sv.insert(sv.cbegin(), 1, 87);

    REQUIRE(sv.size() == 4);
    REQUIRE(sv.capacity() == 4);
    REQUIRE(sv[0] == 87);
    REQUIRE(sv[1] == 12);
    REQUIRE(sv[2] == 45);
    REQUIRE(sv[3] == 98);
}

TEST_CASE("Inserting rvalue with amount 3 in center", "[smolvector]")
{
    mj::SmolVector<int> sv{12, 45, 98};

    sv.insert(sv.cbegin() + 1, 3, 87);

    REQUIRE(sv.size() == 6);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv[0] == 12);
    REQUIRE(sv[1] == 87);
    REQUIRE(sv[2] == 87);
    REQUIRE(sv[3] == 87);
    REQUIRE(sv[4] == 45);
    REQUIRE(sv[5] == 98);
}

TEST_CASE("Inserting rvalue with amount 5 at end", "[smolvector]")
{
    mj::SmolVector<int> sv{12, 45, 98};

    sv.insert(sv.cend(), 5, 87);

    REQUIRE(sv.size() == 8);
    REQUIRE(sv.capacity() == 8);
    REQUIRE(sv[0] == 12);
    REQUIRE(sv[1] == 45);
    REQUIRE(sv[2] == 98);
    REQUIRE(sv[3] == 87);
    REQUIRE(sv[4] == 87);
    REQUIRE(sv[5] == 87);
    REQUIRE(sv[6] == 87);
    REQUIRE(sv[7] == 87);
}

// TODO: emplace.
