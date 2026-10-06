#pragma once

#include <cstddef>
#include <memory>
#include <utility>

// Moves the value leaving the original as 0, but increments the value in the new
// object to prove it has worked.
struct [[nodiscard]] MoveOnly
{
    int moved{};

    [[nodiscard]] MoveOnly() = default;
    [[nodiscard]] MoveOnly(MoveOnly &&o) noexcept : moved{std::exchange(o.moved, 0) + 1} {}
    MoveOnly &operator=(MoveOnly &&o) noexcept
    {
        moved = std::exchange(o.moved, 0) + 1;
        return *this;
    }
    MoveOnly(const MoveOnly &) = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;
};

// Moves the value leaving the original as 0, but increments the value in the new
// object to prove it has worked. Is noexcept moving.
// Also has a copy which increments by 10, for testing containers.
struct [[nodiscard]] CopyAndMoveNoexcept
{
    int moved{};

    [[nodiscard]] CopyAndMoveNoexcept(int initial) { moved = initial; };
    [[nodiscard]] CopyAndMoveNoexcept(CopyAndMoveNoexcept &&o) noexcept : moved{std::exchange(o.moved, 0) + 1} {}
    CopyAndMoveNoexcept &operator=(CopyAndMoveNoexcept &&o) noexcept
    {
        moved = std::exchange(o.moved, 0) + 1;
        return *this;
    }
    CopyAndMoveNoexcept(const CopyAndMoveNoexcept &o) : moved{o.moved + 10} {}
    CopyAndMoveNoexcept &operator=(const CopyAndMoveNoexcept &o)
    {
        moved = o.moved + 10;
        return *this;
    }
};

// Moves the value leaving the original as 0, but increments the value in the new
// object to prove it has worked. Is noexcept moving.
// Also has a copy which increments by 10, for testing containers.
struct [[nodiscard]] CopyAndMove
{
    int moved{};

    [[nodiscard]] CopyAndMove(int initial) { moved = initial; };
    [[nodiscard]] CopyAndMove(CopyAndMove &&o) : moved{std::exchange(o.moved, 0) + 1} {}
    CopyAndMove &operator=(CopyAndMove &&o)
    {
        moved = std::exchange(o.moved, 0) + 1;
        return *this;
    }
    CopyAndMove(const CopyAndMove &o) : moved{o.moved + 10} {}
    CopyAndMove &operator=(const CopyAndMove &o)
    {
        moved = o.moved + 10;
        return *this;
    }
};

struct [[nodiscard]] Counter final
{
    static inline int default_constructed{};
    static inline int value_constructed{};
    static inline int destructored{};
    static inline int copy_constructed{};
    static inline int move_constructed{};
    static inline int copy_assigned{};
    static inline int move_assigned{};

    static void reset()
    {
        default_constructed = value_constructed = destructored = copy_constructed = move_constructed = copy_assigned =
            move_assigned = 0;
    }

    int value{};

    Counter() { ++default_constructed; }
    Counter(int _value) : value{_value} { ++value_constructed; }
    ~Counter() { ++destructored; }
    Counter(const Counter &rhs) : value{rhs.value} { ++copy_constructed; }
    Counter(Counter &&rhs) : value{std::exchange(rhs.value, 0)} { ++move_constructed; }
    Counter &operator=(const Counter &rhs)
    {
        value = rhs.value;
        ++copy_assigned;
        return *this;
    }
    Counter &operator=(Counter &&rhs)
    {
        value = std::exchange(rhs.value, 0);
        ++move_assigned;
        return *this;
    }
};

template <typename T>
struct [[nodiscard]] CountingAllocator final : std::allocator<T>
{
    static inline size_t allocations{};
    static inline size_t deallocations{};
    static inline size_t allocation_calls{};
    static inline size_t deallocation_calls{};

    static void reset() { allocations = deallocations = allocation_calls = deallocation_calls = 0; }

    constexpr T *allocate(size_t n)
    {
        allocations += n;
        allocation_calls++;
        return std::allocator<T>::allocate(n);
    }
    constexpr void deallocate(T *p, size_t n)
    {
        deallocations += n;
        deallocation_calls++;
        std::allocator<T>::deallocate(p, n);
    }
};