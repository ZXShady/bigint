#ifndef ZXSHADY_MATH_HPP
#define ZXSHADY_MATH_HPP

#include "cpp_version.hpp"
#include "macros.hpp"

#include <stdexcept>
#include <type_traits>

namespace zxshady {

template<typename T,typename std::enable_if<std::is_signed<T>::value,int>::type = 0>
constexpr bool is_negative(T t) noexcept
{
    return t < 0;
}

template<typename T,typename std::enable_if<std::is_unsigned<T>::value,int>::type = 0>
constexpr bool is_negative(T) noexcept
{
    return false;
}

template<typename T>
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

template<typename T>
    constexpr T abs(T x) noexcept
    {
        return (is_negative(x)) ? (-x) : x;
    }

    template<typename MinimumType = unsigned char,typename T>
    constexpr typename std::make_unsigned<
        typename std::conditional<(sizeof(MinimumType) > sizeof(T)),
            MinimumType,
            T
        >::type
    >::type unsigned_abs(T x) noexcept
    {
        return (x < 0) ? static_cast<
            typename std::make_unsigned<
                typename std::conditional<(sizeof(MinimumType) > sizeof(T)),
                MinimumType,
                T
                >::type
            >::type
        >(-1 * (x + 1)) + 1 : x;
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

    // 123
    //
    template<typename Integral>
    ZXSHADY_NODISCARD ZXSHADY_CONSTEXPR14 Integral get_nth_digit(Integral x,unsigned long long place,unsigned long long count = 1) noexcept
    {
        static_assert(std::is_integral<Integral>::value, "std::is_integral<Integral>::value");
        return static_cast<Integral>((math::unsigned_abs(x) / math::pow10(place)) % math::pow10(count));
    }
    
    template<typename Integral>
    ZXSHADY_CONSTEXPR14 void set_nth_digit(Integral& x,int new_value,unsigned long long place) noexcept
    {
        assert(new_value >= 0 && new_value <= 9);
        static_assert(std::is_integral<Integral>::value, "std::is_integral<Integral>::value");
        auto digit = math::get_nth_digit(x, place);
        x -= static_cast<Integral>(math::pow10(place) * digit);
        x += static_cast<Integral>(math::pow10(place) * new_value);
    }


    namespace details {
    
    constexpr std::size_t constexpr_log(unsigned long long x, std::size_t ret, std::size_t base = 10)
    {
        return x ? constexpr_log(x / base, ret + 1, base) : ret;
    }

    }
    constexpr std::size_t constexpr_log(unsigned long long x,std::size_t base = 10) noexcept
    {
        return details::constexpr_log(x, 0, base) - 1;
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