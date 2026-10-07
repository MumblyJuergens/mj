#pragma once

#include "mj/math.hpp"
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
struct [[nodiscard]] Smol64
{
    T *inner{};

    Smol64 &move_into(Smol64 &&rhs) noexcept
    {
        inner = std::exchange(rhs.inner, nullptr);
        return *this;
    }

    static constexpr size_t LOWER_48 = 0x0000FFFFFFFFFFFF;

    [[nodiscard]] T *get_ptr() const noexcept
    {
        return reinterpret_cast<T *>(reinterpret_cast<size_t>(inner) & LOWER_48);
    }

    [[nodiscard]] constexpr size_t get_size() const noexcept { return reinterpret_cast<size_t>(inner) >> 48; }

    constexpr void make_ptr(T *const ptr, size_t sz) noexcept
    {
        inner = reinterpret_cast<T *>((reinterpret_cast<size_t>(ptr) & LOWER_48) | (sz << 48));
    }

    [[nodiscard]] static size_t get_capacity(size_t n) noexcept { return next_pow2(n - 1); }
    [[nodiscard]] size_t get_capacity() const noexcept { return get_capacity(get_size()); }

    [[nodiscard]] bool get_empty() const noexcept { return !inner; }
};

template <typename T>
struct [[nodiscard]] Smol32
{
    // As far as I can find, all 32bits are used in the pointer on a modern system.
    T *inner{};
    size_t sizer{};

    Smol32 &move_into(Smol32 &&rhs) noexcept
    {
        inner = std::exchange(rhs.inner, nullptr);
        sizer = std::exchange(rhs.sizer, 0);
        return *this;
    }

    [[nodiscard]] T *get_ptr() const noexcept { return inner; }

    [[nodiscard]] constexpr size_t get_size() const noexcept { return sizer; }

    constexpr void make_ptr(T *const ptr, size_t sz) noexcept
    {
        inner = ptr;
        sizer = sz;
    }

    [[nodiscard]] static size_t get_capacity(size_t n) noexcept { return next_pow2(n - 1); }
    [[nodiscard]] size_t get_capacity() const noexcept { return get_capacity(get_size()); }

    [[nodiscard]] bool get_empty() const noexcept { return !inner; }
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
template <typename T, typename Allocator = std::allocator<T>,
          typename Impl = std::conditional_t<sizeof(size_t) == 8, impl::Smol64<T>, impl::Smol32<T>>>
class [[nodiscard]] SmolVector final : private Impl
{
    [[no_unique_address]] Allocator _allocator;

    constexpr void reset()
    {
        if (Impl::get_empty()) return;
        std::destroy_n(Impl::get_ptr(), size());
        _allocator.deallocate(Impl::get_ptr(), capacity());
        Impl::make_ptr(nullptr, 0);
    }

    constexpr void grow()
    {
        const size_t old_size = size();
        const size_t new_capacity = next_pow2(old_size);
        T *new_inner = _allocator.allocate(new_capacity);
        for (size_t i{}; i < old_size; ++i) {
            // new (new_inner + 1) T{};
            std::construct_at(new_inner + i, std::move_if_noexcept(*(Impl::get_ptr() + i)));
        }
        reset();
        Impl::make_ptr(new_inner, old_size);
        assert(new_inner == Impl::get_ptr());
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
        Impl::move_into(std::move(rhs));
        return *this;
    }

    [[nodiscard]] constexpr SmolVector(const SmolVector &other) { *this = other; }
    constexpr SmolVector &operator=(const SmolVector &rhs)
    {
        reset();
        T *from = rhs.Impl::get_ptr();
        const size_t from_size = rhs.size();
        const size_t from_capacity = next_pow2(from_size);
        T *to = _allocator.allocate(from_capacity);
        for (size_t i{}; i < from_size; ++i) {
            std::construct_at(to + i, *(from + i));
        }
        Impl::make_ptr(to, from_size);
        return *this;
    }

    constexpr ~SmolVector() { reset(); }

    [[nodiscard]] constexpr size_t size() const noexcept { return Impl::get_size(); }
    [[nodiscard]] constexpr int isize() const noexcept { return mj::isize(*this); }
    [[nodiscard]] constexpr size_t capacity() const noexcept { return Impl::get_capacity(); }

    constexpr void push_back(const T &value)
    {
        const auto s = size();
        if (s >= capacity()) {
            grow();
        }
        Impl::make_ptr(Impl::get_ptr(), s + 1);
        *(Impl::get_ptr() + s) = value;
    }

    constexpr void assign(size_t count, const T &value)
    {
        // Because we deduce capacity, we can only use existing memory if the new size
        // would have the same deduced capacity.
        if (Impl::get_capacity(count) != capacity()) {
            reset();
            T *new_inner = _allocator.allocate(Impl::get_capacity(count));
            for (size_t i{}; i < count; ++i) {
                if constexpr (std::is_copy_constructible_v<T>) {
                    std::construct_at(new_inner + i, value);
                } else {
                    std::construct_at(new_inner + i);
                    new_inner[i] = value;
                }
            }
            Impl::make_ptr(new_inner, count);
            return;
        }
        if (size() >= count) {
            for (size_t i = count; i < size(); ++i) {
                std::destroy_at(Impl::get_ptr() + i);
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
                    std::construct_at(Impl::get_ptr() + i, value);
                } else {
                    std::construct_at(Impl::get_ptr() + i);
                    this->at(i) = value;
                }
            }
        }
        Impl::make_ptr(Impl::get_ptr(), count);
    }

    template <typename InputIt>
        requires std::input_iterator<InputIt>
    constexpr void assign(InputIt first, InputIt last)
    {
        const auto count = static_cast<size_t>(std::distance(first, last));
        // Because we deduce capacity, we can only use existing memory if the new size
        // would have the same deduced capacity.
        if (Impl::get_capacity(count) != capacity()) {
            reset();
            T *new_inner = _allocator.allocate(Impl::get_capacity(count));
            for (size_t i{}; i < count; ++i) {
                if constexpr (std::is_copy_constructible_v<T>) {
                    std::construct_at(new_inner + i, *first++);
                } else {
                    std::construct_at(new_inner + i);
                    new_inner[i] = first++;
                }
            }
            Impl::make_ptr(new_inner, count);
            return;
        }
        if (size() >= count) {
            for (size_t i = count; i < size(); ++i) {
                std::destroy_at(Impl::get_ptr() + i);
            }
            for (size_t i{}; i < count; ++i) {
                this->at(i) = *first++;
            }
        } else {
            for (size_t i{}; i < size(); ++i) {
                this->at(i) = *first++;
            }
            for (size_t i = size(); i < count; ++i) {
                if constexpr (std::is_copy_constructible_v<T>) {
                    std::construct_at(Impl::get_ptr() + i, *first++);
                } else {
                    std::construct_at(Impl::get_ptr() + i);
                    this->at(i) = *first++;
                }
            }
        }
        Impl::make_ptr(Impl::get_ptr(), static_cast<size_t>(count));
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return _allocator; }

    [[nodiscard]] constexpr reference at(size_type n) { return *(Impl::get_ptr() + n); }
    [[nodiscard]] constexpr const_reference at(size_type n) const { return *(Impl::get_ptr() + n); }

    [[nodiscard]] constexpr pointer data() noexcept { return Impl::get_ptr(); }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return Impl::get_ptr(); }

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

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator{Impl::get_ptr()}; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator{Impl::get_ptr()}; }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return const_iterator{Impl::get_ptr()}; }
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{end()}; }
    [[nodiscard]] constexpr iterator end() noexcept { return iterator{Impl::get_ptr() + Impl::get_size()}; }
    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return const_iterator{Impl::get_ptr() + Impl::get_size()};
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return const_iterator{Impl::get_ptr() + Impl::get_size()};
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
