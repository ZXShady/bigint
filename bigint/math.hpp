#ifndef ZXSHADY_MATH_HPP
#define ZXSHADY_MATH_HPP

#include "cpp_version.hpp"
#include "macros.hpp"

#include <stdexcept>
#include <type_traits>

namespace zxshady {
template<class T,typename std::enable_if<std::is_signed<T>::value,int>::type = 0>
constexpr bool is_negative(T t) noexcept
{
    return t < 0;
}

template<class T,typename std::enable_if<std::is_unsigned<T>::value,int>::type = 0>
constexpr bool is_negative(T t) noexcept
{
    return false;
}

template<class T>
constexpr bool is_positive(T t) noexcept
{
    return !is_negative(t);
}

namespace math {
template<typename T>
    ZXSHADY_NODISCARD ZXSHADY_CONSTEXPR14 std::size_t digit_count(T x) noexcept
    {
        std::size_t cnt{};
        do
            cnt++;
        while (x /= 10);
        return cnt;
    }

template<class T>
    constexpr T abs(T x) noexcept
    {
        return (is_negative(x)) ? (-x) : x;
    }

    template<class T>
    constexpr typename std::make_unsigned<T>::type unsigned_abs(T x) noexcept
    {
        return (x < 0) ? static_cast<typename std::make_unsigned<T>::type>(-1 * (x + 1)) + 1 : x;
    }

    template<typename T>
    ZXSHADY_NODISCARD ZXSHADY_CONSTEXPR14 T pow(T base, std::size_t exponent) noexcept
    {
        if (exponent == 0) return 1;
        const auto mul = base;
        while (exponent != 1) {
            base *= mul;
            --exponent;
        }
        return base;
    }
    constexpr unsigned long long pow2(unsigned long long exponent)
    {
        return 1ULL << exponent;
    }

    ZXSHADY_CONSTEXPR14 unsigned long long pow10(unsigned long long exponent) noexcept
    {
        constexpr unsigned long long mPow10Table[] = {
            1,
            10,
            100,
            1000,
            10000,
            100000,
            1000000,
            10000000,
            100000000,
            1000000000,
            10000000000,
            100000000000,
            1000000000000,
            10000000000000,
            100000000000000,
            1000000000000000,
            10000000000000000
        };
        return mPow10Table[exponent];
    }

    template<typename Integral>
    ZXSHADY_NODISCARD ZXSHADY_CONSTEXPR14 Integral nth_digit(Integral x,unsigned long long place,unsigned long long count = 1) noexcept
    {
        static_assert(std::is_integral<Integral>::value, "std::is_integral<Integral>::value");
        return math::unsigned_abs(x) / math::pow10(math::digit_count(x) - place) % math::pow10(count);
    }



    template<typename Ret = std::size_t, typename T>
    ZXSHADY_NODISCARD ZXSHADY_CONSTEXPR14 Ret log(T x,std::size_t base){
        if (x == 0)
            throw std::invalid_argument("zxshady::math::log() log of 0 is not defined");

        if (is_negative(x))
            throw std::invalid_argument("zxshady::math::log() log of a negative number is not defined");

        Ret ret = 0;
        while (x) {
            ++ret;
            x /= static_cast<T>(base);
        }
        return ret-1;
    }

}

}

#endif