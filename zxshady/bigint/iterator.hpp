#ifndef ZXSHADY_BIGINT_ITERATOR_HPP
#define ZXSHADY_BIGINT_ITERATOR_HPP

#include "../../zxshady/macros.hpp"
#include "../../zxshady/math.hpp"
#include "reference.hpp"
#include <type_traits>

namespace zxshady {
namespace details {
namespace bigint {

template<typename T, unsigned long long MaxDigits>
class iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::remove_cv<T>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = ::zxshady::details::bigint::reference<T>;

    constexpr iterator() noexcept : m_ptr(nullptr), m_place(0) {}
    constexpr iterator(T* ptr, int place) noexcept : m_ptr(ptr), m_place(place) {}

    constexpr iterator(const iterator&) noexcept = default;
    constexpr iterator& operator=(const iterator&) &noexcept = default;


    iterator& operator++() & noexcept
    {
        ++m_place;
        if (static_cast<unsigned long long>(m_place) >= MaxDigits) {
            m_ptr++;
            m_place = 0;
        }

        return *this;
    }

    ZXSHADY_NODISCARD iterator operator++(int) &noexcept
    {
        iterator copy = *this;
        ++(*this);
        return copy;
    }

    iterator& operator--() & noexcept
    {
        --m_place;
        if (m_place == -1) {
            m_ptr--;
            m_place = MaxDigits - 1;
        }

        return *this;
    }

    ZXSHADY_NODISCARD iterator operator--(int) noexcept
    {
        iterator copy = *this;
        --(*this);
        return copy;
    }

    reference operator*() const noexcept
    {
        return reference(m_ptr, m_place);
    }

    ZXSHADY_NODISCARD constexpr bool operator==(iterator that) const noexcept
    {
        return m_place == that.m_place && m_ptr == that.m_ptr;
    }


    template<typename T_, typename U_, unsigned long long Max>
    friend ZXSHADY_NODISCARD constexpr bool operator==(iterator<T_, Max> a, iterator<U_, Max> b) noexcept;

private:
    T* m_ptr;
    int m_place;
};

template<typename T, typename U, unsigned long long Max>
ZXSHADY_NODISCARD constexpr bool operator==(iterator<T, Max> a, iterator<U, Max> b) noexcept
{
    return a.m_place == b.m_place && a.m_ptr == b.m_ptr;
}

template<typename T, typename U, unsigned long long Max>
ZXSHADY_NODISCARD constexpr bool operator!=(iterator<T, Max> a, iterator<U, Max> b) noexcept
{
    return !(a == b);
}

template<typename T,typename U,unsigned long long Max>
ZXSHADY_NODISCARD constexpr bool operator<(iterator<T, Max> a, iterator<U, Max> b) noexcept
{
    return a.m_ptr < b.m_ptr || a.m_place < b.m_place;
}

template<typename T,typename U,unsigned long long Max>
ZXSHADY_NODISCARD constexpr bool operator>(iterator<T, Max> a, iterator<U, Max> b) noexcept
{
    return b < a;
}

template<typename T,typename U,unsigned long long Max>
ZXSHADY_NODISCARD constexpr bool operator<=(iterator<T, Max> a, iterator<U, Max> b) noexcept
{
    return !(a > b);
}

template<typename T,typename U,unsigned long long Max>
ZXSHADY_NODISCARD constexpr bool operator>=(iterator<T, Max> a, iterator<U, Max> b) noexcept
{
    return !(a < b);
}

}
}
}

#endif // !defined(ZXSHADY_BIGINT_ITERATOR_HPP)