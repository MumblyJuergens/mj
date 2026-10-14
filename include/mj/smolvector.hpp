#pragma once

#include "mj/algorithm.hpp"
#include "mj/math.hpp"
#include "mj/size.hpp"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
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

    [[nodiscard]] constexpr static size_t get_capacity(size_t n) noexcept { return next_pow2(n - 1); }
    [[nodiscard]] size_t get_capacity() const noexcept { return get_capacity(get_size()); }

    [[nodiscard]] bool get_empty() const noexcept { return !inner; }

    [[nodiscard]] constexpr size_t get_max_size() const noexcept { return std::numeric_limits<uint16_t>::max(); }
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

    [[nodiscard]] size_t get_max_size() const noexcept { return std::numeric_limits<size_t>::max(); }
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
///
/// While all attempts are made to be a drop in replacement for std::vector
/// some methods have to be missing because capacity is deduced. These are:
///
/// reserve(size_type)
/// shrink_to_fit()
///
/// Despite markings, the class is not necessarily constexpr. This depends
/// on the `Impl` class used to create it, by default on 64bit machines
/// this does bit manipulation on pointers and can't be constexpr.
template <typename T, typename Allocator = std::allocator<T>,
          typename Impl = std::conditional_t<sizeof(size_t) == 8, impl::Smol64<T>, impl::Smol32<T>>>
class [[nodiscard]] SmolVector final : private Impl
{
    [[no_unique_address]] Allocator _allocator;

    constexpr void reset()
    {
        if (Impl::get_empty()) return;
        const auto s = size();
        const auto c = capacity();
        std::destroy_n(Impl::get_ptr(), s);
        _allocator.deallocate(Impl::get_ptr(), c);
        Impl::make_ptr(nullptr, 0);
    }

    /**
     * @brief Grows the storage to the requested size.
     *
     * Note that after this method, the capacity deduced may be utterly wrong for the size
     * value. That is the calling methods problem.
     *
     * @param needed The minimum capacity required. Leaving as the default nullopt
     * is safest for most uses, causes growing to the next size up.
     */
    constexpr void grow(std::optional<size_t> needed = std::nullopt)
    {
        const size_t old_size = size();
        const size_t new_capacity = next_pow2(needed.value_or(old_size));
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

    [[nodiscard]] constexpr SmolVector() noexcept(noexcept(Allocator{})) = default;

    [[nodiscard]] constexpr explicit SmolVector(const Allocator &alloc) noexcept : _allocator{alloc} {}

    [[nodiscard]] constexpr explicit SmolVector(size_type count, const Allocator &alloc = Allocator{})
        : _allocator{alloc}
    {
        const size_t new_capacity = Impl::get_capacity(count);
        T *new_inner = _allocator.allocate(new_capacity);
        for (size_t i{}; i < count; ++i) {
            std::construct_at(new_inner + i);
        }
        Impl::make_ptr(new_inner, count);
        assert(new_inner == Impl::get_ptr());
    }

    [[nodiscard]] constexpr SmolVector(size_type count, const T &value, const Allocator &alloc = Allocator{})
        : _allocator{alloc}
    {
        const size_t new_capacity = Impl::get_capacity(count);
        T *new_inner = _allocator.allocate(new_capacity);
        for (size_t i{}; i < count; ++i) {
            std::construct_at(new_inner + i, value);
        }
        Impl::make_ptr(new_inner, count);
        assert(new_inner == Impl::get_ptr());
    }

    template <typename InputIt>
    [[nodiscard]] constexpr SmolVector(InputIt first, InputIt last, const Allocator &alloc = Allocator{})
        : _allocator{alloc}
    {
        const size_t count = static_cast<size_t>(std::distance(first, last));
        const size_t new_capacity = Impl::get_capacity(count);
        T *new_inner = _allocator.allocate(new_capacity);
        for (size_t i{}; i < count; ++i) {
            std::construct_at(new_inner + i, *first++);
        }
        Impl::make_ptr(new_inner, count);
        assert(new_inner == Impl::get_ptr());
    }

    template <typename Range>
    [[nodiscard]] constexpr SmolVector(std::from_range_t, Range &&rg, const Allocator &alloc = Allocator{})
        : SmolVector{std::ranges::begin(rg), std::ranges::end(rg), alloc}
    {
    }

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
        assign_range(rhs);
        return *this;
    }

    [[nodiscard]] constexpr SmolVector(const SmolVector &other, const std::type_identity_t<Allocator> &alloc)
        : _allocator{alloc}
    {
        *this = other;
    }

    [[nodiscard]] constexpr SmolVector(SmolVector &&other, const std::type_identity_t<Allocator> &alloc) noexcept
        : _allocator{alloc}
    {
        *this = std::move(other);
    }

    constexpr SmolVector &operator=(std::initializer_list<T> list)
    {
        assign(list);
        return *this;
    }

    [[nodiscard]] constexpr SmolVector(std::initializer_list<T> list, const Allocator &alloc = Allocator{})
        : _allocator{alloc}
    {
        *this = list;
    }

    constexpr ~SmolVector() { reset(); }

    [[nodiscard]] constexpr size_t size() const noexcept { return Impl::get_size(); }
    [[nodiscard]] constexpr int isize() const noexcept { return mj::isize(*this); }
    [[nodiscard]] constexpr size_t max_size() const noexcept { return Impl::get_max_size(); }
    [[nodiscard]] constexpr size_t capacity() const noexcept { return Impl::get_capacity(); }

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

    constexpr void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }

    template <typename Range>
    constexpr void assign_range(Range &&range)
    {
        assign(std::ranges::begin(range), std::ranges::end(range));
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return _allocator; }

    [[nodiscard]] constexpr reference at(size_type n)
    {
        if (n >= size()) {
            throw std::out_of_range{"Item out of range in SmolVector"};
        }
        return *(Impl::get_ptr() + n);
    }

    [[nodiscard]] constexpr const_reference at(size_type n) const
    {
        if (n >= size()) {
            throw std::out_of_range{"Item out of range in SmolVector"};
        }
        return *(Impl::get_ptr() + n);
    }

    [[nodiscard]] constexpr reference operator[](size_type n) { return *(Impl::get_ptr() + n); }
    [[nodiscard]] constexpr const_reference operator[](size_type n) const { return *(Impl::get_ptr() + n); }

    [[nodiscard]] constexpr reference front() { return *(Impl::get_ptr()); }
    [[nodiscard]] constexpr const_reference front() const { return *(Impl::get_ptr()); }

    [[nodiscard]] constexpr reference back() { return *(Impl::get_ptr() + size() - 1); }
    [[nodiscard]] constexpr const_reference back() const { return *(Impl::get_ptr() + size() - 1); }

    [[nodiscard]] constexpr pointer data() noexcept { return Impl::get_ptr(); }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return Impl::get_ptr(); }

    [[nodiscard]] constexpr bool empty() const noexcept { return Impl::get_empty(); }

    constexpr void clear() noexcept { reset(); }

    class Iterator final
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

    constexpr void push_back(const T &value)
    {
        const auto s = size();
        if (s >= capacity()) {
            grow();
        }
        std::construct_at(Impl::get_ptr() + s, value);
        Impl::make_ptr(Impl::get_ptr(), s + 1);
    }

    constexpr iterator insert(const_iterator position, const T &value) { return emplace(position, value); }
    constexpr iterator insert(const_iterator position, T &&value) { return emplace(position, std::move(value)); }

    constexpr iterator insert(const_iterator position, size_type count, const T &value)
    {
        // Note: Nearly identical to some other insert/emplace methods, keep all updated.
        const auto current_size = size();
        const auto current_capacity = capacity();
        const auto future_capacity = Impl::get_capacity(current_size + count);
        const difference_type pos = current_size == 0 ? 0 : position - cbegin();
        T *start = Impl::get_ptr();
        T *target = start;

        if (current_capacity != future_capacity) {
            target = _allocator.allocate(future_capacity);
            // Move any items before pos to the same position in the new memory.
            move_if_noexcept_construct(start, start + pos, target);
            // Move any items after pos leaving space for new items.
            move_if_noexcept_construct(start + pos, start + current_size, target + pos + count);
            // And construct the new items in the gap.
            construct_n_at(target + pos, target + pos + count, value);
            reset();
        } else {
            // Move items after the position, constructing if empty space.
            move_if_noexcept_construct_reverse(start + pos, start + current_size, target + current_size + count);
            // And construct the new items in the gap.
            construct_n_at(target + pos, target + pos + count, value);
        }

        Impl::make_ptr(target, current_size + count);
        return target + pos;
    }

    template <typename OutputIt, typename... Args>
    OutputIt construct_n_at(OutputIt first, OutputIt last, Args &&...args)
    {
        for (; first != last; ++first) {
            std::construct_at(first, std::forward<Args>(args)...);
        }
        return first;
    }

    template <typename... Args>
    constexpr iterator emplace(const_iterator position, Args &&...args)
    {
        // Note: Nearly identical to some other insert/emplace methods, keep all updated.
        const auto s = size();
        if (s >= capacity()) {
            grow();
        }
        const difference_type pos = s == 0 ? 0 : position - cbegin();
        T *start = Impl::get_ptr();
        // If we had any items, move them along.
        // Move item at end to the new end first.
        if (s != 0) {
            std::construct_at(start + s, std::move_if_noexcept(*(start + s - 1)));
            // Move the rest along one.
            for (auto it = start + s - 1; it > start + pos; --it) {
                std::destroy_at(it);
                std::construct_at(it, std::move_if_noexcept(*(it - 1)));
            }
        }
        // Set the last moved-from at position to value.
        std::destroy_at(start + pos);
        std::construct_at(start + pos, std::forward<Args>(args)...);

        Impl::make_ptr(Impl::get_ptr(), s + 1);
        return start + pos;
    }
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
