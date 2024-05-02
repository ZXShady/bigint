#ifndef ZXSHADY_MACROS_HPP
#define ZXSHADY_MACROS_HPP
#include "cpp_version.hpp"

#define ZXSHADY_XCONCAT(a,b) a ## b
#define ZXSHADY_CONCAT(a,b) ZXSHADY_XCONCAT(a,b)

#ifdef ZXSHADY_CPP11
#define ZXSHADY_CONSTEXPR11 constexpr
#else // !defined(ZXSHADY_CPP14)
#define ZXSHADY_CONSTEXPR11
#endif


#ifdef ZXSHADY_CPP14
#define ZXSHADY_CONSTEXPR14 constexpr
#else // !defined(ZXSHADY_CPP14)
#define ZXSHADY_CONSTEXPR14
#endif

#ifdef ZXSHADY_CPP17
#define ZXSHADY_NODISCARD [[nodiscard]]
#define ZXSHADY_CONSTEXPR17 constexpr
#else // !defined(ZXSHADY_CPP17)
#define ZXSHADY_CONSTEXPR17
#define ZXSHADY_NODISCARD
#endif

#if ZXSHADY_CPP_CURRENT_VERSION >= 20
#define ZXSHADY_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else // !(ZXSHADY_CPP_CURRENT_VERSION >= 20)
#define ZXSHADY_NODISCARD_MSG(msg) ZXSHADY_NODISCARD
#endif


#define ZXSHADY_CONSTEXPR(version) ZXSHADY_CONCAT(ZXSHADY_CONSTEXPR,version)


#define ZXSHADY_DEFINE_COMPARISONS(type1,type2) \
ZXSHADY_NODISCARD inline bool operator>(type1 a,type2 b) noexcept { return b < a;} \
ZXSHADY_NODISCARD inline bool operator<=(type1 a,type2 b) noexcept { return !(a > b);} \
ZXSHADY_NODISCARD inline bool operator>=(type1 a,type2 b) noexcept { return !(a < b);} \
ZXSHADY_NODISCARD inline bool operator!=(type1 a,type2 b) noexcept { return !(a == b);}


#define ZXSHADY_DEFINE_ARITHMETIC(type1,type2) \
ZXSHADY_NODISCARD inline type1 operator+(type1 a,type2 b) { return a += b;}\
ZXSHADY_NODISCARD inline type1 operator-(type1 a,type2 b) { return a -= b;}\
ZXSHADY_NODISCARD inline type1 operator*(type1 a,type2 b) { return a *= b;}\
ZXSHADY_NODISCARD inline type1 operator/(type1 a,type2 b) { return a /= b;}\
ZXSHADY_NODISCARD inline type1 operator%(type1 a,type2 b) { return a %= b;}


#endif // !defined(ZXSHADY_MACROS_HPP)