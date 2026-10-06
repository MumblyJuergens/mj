#pragma once

#include "mj/size.hpp"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace mj
{

namespace impl
{

template <typename T>
struct [[nodiscard]] Smol64 final
{
    T *inner{};

    static constexpr size_t LOWER_48 = 0x0000FFFFFFFFFFFF;

    [[nodiscard]] T *get_ptr() const noexcept
    {
        return reinterpret_cast<T *>(reinterpret_cast<size_t>(inner) & LOWER_48);
    }

    [[nodiscard]] constexpr size_t get_size() noexcept { return reinterpret_cast<size_t>(inner) >> 48; }

    constexpr void make_ptr(T *const ptr, size_t sz) noexcept
    {
        inner = reinterpret_cast<T *>((reinterpret_cast<size_t>(ptr) & LOWER_48) | (sz << 48));
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

    [[nodiscard]] size_t get_capacity() noexcept { return next_pow2(get_size() - 1); }
};

} // namespace impl

/// @brief A small vector for cache optimisation.
///
/// Basically, modern 64bit CPU's only actually use pointers that
/// occupy 48bits. This leaves us with 16 bits to store information,
/// in this case the size of the vector. Assuming you don't step over
/// 65k items, this is fine.
///
/// A std::vector takes 3x64bits for a data, size, and capacity value.
/// Since capacity always doubles when size reaches capacity, we can
/// calculate capacity based on the next power of two and avoid storing
/// it, at the cost of a an occasional calculation.
template <typename T, typename Allocator = std::allocator<T>>
class [[nodiscard]] SmolVector final
{
    T *inner{};
    [[no_unique_address]] Allocator _allocator;

    static constexpr size_t LOWER_48 = 0x0000FFFFFFFFFFFF;

    [[nodiscard]] static constexpr T *get_ptr(T *const ptr) noexcept
    {
        return reinterpret_cast<T *>(reinterpret_cast<size_t>(ptr) & LOWER_48);
    }

    [[nodiscard]] static constexpr size_t get_size(T *const ptr) noexcept
    {
        return reinterpret_cast<size_t>(ptr) >> 48;
    }

    [[nodiscard]] static constexpr T *make_ptr(T *const ptr, size_t sz) noexcept
    {
        return reinterpret_cast<T *>((reinterpret_cast<size_t>(ptr) & LOWER_48) | (sz << 48));
    }

    constexpr void reset()
    {
        if (!inner) return;
        std::destroy_n(get_ptr(inner), size());
        _allocator.deallocate(get_ptr(inner), capacity());
        inner = nullptr;
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

    [[nodiscard]] constexpr static size_t get_capacity(size_t s) noexcept { return next_pow2(s - 1); }

    constexpr void grow()
    {
        const size_t old_size = size();
        const size_t new_capacity = next_pow2(old_size);
        T *new_inner = _allocator.allocate(new_capacity);
        for (size_t i{}; i < old_size; ++i) {
            // new (new_inner + 1) T{};
            std::construct_at(new_inner + i, std::move_if_noexcept(*(get_ptr(inner) + i)));
        }
        reset();
        inner = make_ptr(new_inner, old_size);
        assert(new_inner == get_ptr(inner));
    }

  public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T &;
    using const_reference = const T &;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    [[nodiscard]] constexpr SmolVector() noexcept = default;

    [[nodiscard]] constexpr SmolVector(SmolVector &&other) noexcept { *this = std::move(other); }
    constexpr SmolVector &operator=(SmolVector &&rhs) noexcept
    {
        reset();
        inner = std::exchange(rhs.inner, nullptr);
        return *this;
    }

    [[nodiscard]] constexpr SmolVector(const SmolVector &other) { *this = other; }
    constexpr SmolVector &operator=(const SmolVector &rhs)
    {
        reset();
        T *from = get_ptr(rhs.inner);
        const size_t from_size = rhs.size();
        const size_t from_capacity = next_pow2(from_size);
        T *to = _allocator.allocate(from_capacity);
        for (size_t i{}; i < from_size; ++i) {
            std::construct_at(to + i, *(from + i));
        }
        inner = make_ptr(to, from_size);
        return *this;
    }

    constexpr ~SmolVector() { reset(); }

    [[nodiscard]] constexpr size_t size() const noexcept { return get_size(inner); }
    [[nodiscard]] constexpr int isize() const noexcept { return mj::isize(*this); }
    [[nodiscard]] constexpr size_t capacity() const noexcept { return get_capacity(size()); }

    constexpr void push_back(const T &value)
    {
        const auto s = size();
        if (s >= capacity()) {
            grow();
        }
        inner = make_ptr(get_ptr(inner), s + 1);
        *(get_ptr(inner) + s) = value;
    }

    constexpr void assign(size_t count, const T &value)
    {
        // Because we deduce capacity, we can only use existing memory if the new size
        // would have the same deduced capacity.
        if (get_capacity(count) != capacity()) {
            reset();
            T *new_inner = _allocator.allocate(get_capacity(count));
            for (size_t i{}; i < count; ++i) {
                if constexpr (std::is_copy_constructible_v<T>) {
                    std::construct_at(new_inner + i, value);
                } else {
                    std::construct_at(new_inner + i);
                    new_inner[i] = value;
                }
            }
            inner = make_ptr(new_inner, count);
            return;
        }
        if (size() >= count) {
            for (size_t i = count; i < size(); ++i) {
                std::destroy_at(get_ptr(inner) + i);
            }
            for (size_t i{}; i < count; ++i) {
                this->at(i) = value;
            }
        } else {
            for (size_t i{}; i < size(); ++i) {
                this->at(i) = value;
            }
            for (size_t i = size(); i < count; ++i) {
                if constexpr (std::is_copy_constructible_v<T>) {
                    std::construct_at(get_ptr(inner) + i, value);
                } else {
                    std::construct_at(get_ptr(inner) + i);
                    this->at(i) = value;
                }
            }
        }
        inner = make_ptr(get_ptr(inner), count);
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return _allocator; }

    [[nodiscard]] constexpr reference at(size_type n) { return *(get_ptr(inner) + n); }
    [[nodiscard]] constexpr const_reference at(size_type n) const { return *(get_ptr(inner) + n); }

    [[nodiscard]] constexpr pointer data() noexcept { return get_ptr(inner); }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return get_ptr(inner); }

    class [[nodiscard]] Iterator final
    {
        T *it{};

      public:
        using difference_type = SmolVector<T>::difference_type;
        using value_type = SmolVector::value_type;
        using element_type = T;
        using iterator_category = std::contiguous_iterator_tag;

        [[nodiscard]] constexpr Iterator() noexcept = default;
        [[nodiscard]] constexpr Iterator(T *_it) noexcept : it{_it} {}
        constexpr T &operator*() const noexcept { return *it; }
        constexpr Iterator &operator++() noexcept
        {
            ++it;
            return *this;
        }
        constexpr Iterator operator++(int) noexcept
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        constexpr Iterator &operator--() noexcept
        {
            --it;
            return *this;
        }
        constexpr Iterator operator--(int) noexcept
        {
            auto tmp = *this;
            --*this;
            return tmp;
        }
        constexpr bool operator==(const Iterator &rhs) const noexcept { return it == rhs.it; }
        constexpr auto operator<=>(const Iterator &) const noexcept = default;
        constexpr ptrdiff_t operator-(const Iterator &rhs) const noexcept { return it - rhs.it; }
        constexpr friend Iterator operator+(const Iterator &i, ptrdiff_t n) noexcept { return Iterator{i.it + n}; }
        constexpr friend Iterator operator+(ptrdiff_t n, const Iterator &i) noexcept { return Iterator{i.it + n}; }
        constexpr Iterator operator-(ptrdiff_t n) const noexcept { return Iterator{it - n}; }
        constexpr Iterator &operator+=(ptrdiff_t n) noexcept
        {
            it += n;
            return *this;
        }
        constexpr Iterator &operator-=(ptrdiff_t n) noexcept
        {
            it -= n;
            return *this;
        }
        constexpr T &operator[](ptrdiff_t n) const noexcept { return it[n]; }
        constexpr T *operator->() const noexcept { return it; }
        constexpr friend T *address_of(Iterator &p) noexcept { return p.it; }
    };

    using iterator = Iterator;
    using const_iterator = SmolVector<const T>::Iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{get_ptr(inner)}; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{get_ptr(inner)}; }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return const_iterator{get_ptr(inner)}; }
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{end()}; }
    [[nodiscard]] constexpr iterator end() noexcept { return iterator{get_ptr(inner) + get_size(inner)}; }
    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return const_iterator{get_ptr(inner) + get_size(inner)};
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return const_iterator{get_ptr(inner) + get_size(inner)};
    }
    [[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{begin()}; }
};

static_assert(std::weakly_incrementable<SmolVector<int>::Iterator>);
static_assert(std::input_or_output_iterator<SmolVector<int>::Iterator>);
static_assert(std::input_iterator<SmolVector<int>::Iterator>);
static_assert(std::forward_iterator<SmolVector<int>::Iterator>);
static_assert(std::bidirectional_iterator<SmolVector<int>::Iterator>);
static_assert(std::totally_ordered<SmolVector<int>::Iterator>);
static_assert(std::sized_sentinel_for<SmolVector<int>::Iterator, SmolVector<int>::Iterator>);
static_assert(std::random_access_iterator<SmolVector<int>::Iterator>);
static_assert(std::contiguous_iterator<SmolVector<int>::Iterator>);
static_assert(sizeof(SmolVector<int>) == sizeof(size_t));

} // namespace mj