
#ifndef ZXSHADY_VERSION_CPP_VERSION_HPP
#define ZXSHADY_VERSION_CPP_VERSION_HPP

#if defined(_MSC_VER) && !defined(__clang__)
    #define ZXSHADY_CPP_VER _MSVC_LANG 
#else
    #define ZXSHADY_CPP_VER __cplusplus
#endif

#if ZXSHADY_CPP_VER >=  201103L
    #define ZXSHADY_CPP11
#if ZXSHADY_CPP_VER == 201103L  
#define ZXSHADY_CPP_CURRENT_VERSION 11
#endif

#endif

#if ZXSHADY_CPP_VER >=  201402L
    #define ZXSHADY_CPP14
    
    #if ZXSHADY_CPP_VER == 201402L  
    #define ZXSHADY_CPP_CURRENT_VERSION 14
    #endif

#endif

#if ZXSHADY_CPP_VER >= 201703L
    #define ZXSHADY_CPP17
    
    #if ZXSHADY_CPP_VER == 201703L  
    #define ZXSHADY_CPP_CURRENT_VERSION 17
    #endif


#endif

#if ZXSHADY_CPP_VER >= 202002L
    #define ZXSHADY_CPP20
    #if ZXSHADY_CPP_VER == 202002L  
    #define ZXSHADY_CPP_CURRENT_VERSION 20
    #endif
    
#endif

#if ZXSHADY_CPP_VER >= 202302L
    #define ZXSHADY_CPP23
    #if ZXSHADY_CPP_VER == 202302L  
    #define ZXSHADY_CPP_CURRENT_VERSION 23
    #endif
#endif

#ifdef ZXSHADY_CPP14

#define ZXSHADY_CONSTEXPR14 constexpr
#else
#define ZXSHADY_CONSTEXPR14
#endif

#ifdef ZXSHADY_CPP17

#define ZXSHADY_NODISCARD [[nodiscard]]
#define ZXSHADY_CONSTEXPR17 constexpr
#else
#define ZXSHADY_CONSTEXPR17
#define ZXSHADY_NODISCARD
#endif

#if ZXSHADY_CPP_CURRENT_VERSION >= 20

#define ZXSHADY_NODISCARD_MSG(msg) [[nodiscard(msg)]]

#else

#define ZXSHADY_NODISCARD_MSG(msg) ZXSHADY_NODISCARD

#endif



#endif // !ZXSHADY_VERSION_CPP_VERSION_HPP



#ifdef ZXSHADY_NO_EXCEPTIONS

#define ZXSHADY_THROW(type) throw type;

#else

#define ZXSHADY_THROW(type) assert(false);

#endif



#ifndef ZXSHADY_BIGINT_HPP
#define ZXSHADY_BIGINT_HPP

#include <iosfwd>

#include <string>
#include <vector>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <utility>

#include <cstddef>
#include <cstring>
#include <cassert>
#include <system_error>

#ifdef ZXSHADY_CPP20

#include <compare>

#endif

namespace zxshady {

// copied from cpp.reference
// https://en.cppreference.com/w/cpp/utility/unreachable
[[noreturn]] inline void unreachable()
{
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

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

class math {
public:
    math() = delete;
    math(const math&) = delete;
    math(math&&) = delete;
    void operator=(const math&) = delete;
    void operator=(math&&) = delete;
    ~math() = delete;


    template<class T>
    static constexpr T abs(T x) noexcept
    {
        return (is_negative(x)) ? (-x) : x;
    }

    template<class T>
    static constexpr typename std::make_unsigned<T>::type unsigned_abs(T x) noexcept
    {
        return (x < 0) ? static_cast<typename std::make_unsigned<T>::type>(-1 * (x + 1)) + 1 : x;
    }

    static constexpr unsigned long long pow2(unsigned long long exponent)
    {
        return 1ULL << exponent;
    }

    static ZXSHADY_CONSTEXPR14 unsigned long long pow10(unsigned long long exponent) noexcept
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

    template<class Integral>
    static ZXSHADY_CONSTEXPR14 Integral nth_digit(Integral x,unsigned long long place,unsigned long long count = 1) noexcept
    {
        static_assert(std::is_integral<Integral>::value, "std::is_integral<Integral>::value");
        return math::unsigned_abs(x) / math::pow10(math::digit_count(x) - place) % math::pow10(count);
    }


    template<class T>
    static ZXSHADY_CONSTEXPR14 std::size_t digit_count(T x) noexcept
    {
        std::size_t cnt{};
        do
            cnt++;
        while (x /= 10);
        return cnt;
    }
private:
    
};
template<typename T,typename E = std::errc>
struct result {
    T val;
    E ec{};
    constexpr bool operator!() const noexcept
    {
        return ec != E{};
    }

    constexpr explicit operator bool() const noexcept
    {
        return !!(*this);
    }



};
template<class T> struct is_signed_integer   : public std::integral_constant<bool,std::is_integral<T>::value && std::is_signed<T>::value > {};
template<class T> struct is_unsigned_integer : public std::integral_constant<bool,std::is_integral<T>::value && std::is_unsigned<T>::value > {};

enum noinit_t {};
class bigint final {
private: /********PRIVATE CONSTRUCTORS ***********/


    bigint(noinit_t, bool is_negative = false) noexcept
        : mIsNegative(is_negative)
    {
    }
public:
    using number_type = std::uint32_t;

    ////////////////////////////////////////////////////////////
    /// \brief a constructor that initializes the bigint to 0
    ////////////////////////////////////////////////////////////
    bigint()
        :mNumbers({ 0 })
        , mIsNegative(false)
    {
    }

    bigint(const bigint&) = default;
    bigint(bigint&&) noexcept = default;
    bigint& operator=(const bigint&) & = default;
    bigint& operator=(bigint&&) &noexcept = default;
    ~bigint() noexcept = default;

    template<class Integer, typename std::enable_if<
        std::is_integral<Integer>::value, int>::type = 0>
    bigint(Integer num) : mIsNegative(false)
    {
        if (::zxshady::is_negative(num)) {
            mIsNegative = true;
            num = math::unsigned_abs(num);
        }

        if ZXSHADY_CONSTEXPR17(sizeof(num) < sizeof(number_type))
        {
            mNumbers.push_back(num);
        }
        else {
            while (num != 0) {
                mNumbers.push_back(num % kMaxDigitsInNumber);
                num /= kMaxDigitsInNumber;
            }
        }

        if (mNumbers.empty())
            mNumbers.push_back(0);
    }


    bigint(std::nullptr_t) = delete;
    bigint(const char* begin, const char* end);
    bigint(const char* str, std::size_t size) : bigint(str, str + size) {}
    explicit bigint(const std::string& str) : bigint(str.c_str(), str.c_str() + str.size()) {}
    explicit bigint(const char* numStr) : bigint(numStr, numStr + std::strlen(numStr)) {}


    bigint& operator=(const char* s)&
    {
        return *this = bigint{ s };
    }

    bigint& operator=(const std::string& s)&
    {
        return *this = bigint{ s };
    }

    template<class Integral, typename std::enable_if<
        std::is_integral<Integral>::value, int>::type = 0>
    bigint& operator=(Integral num) & noexcept(sizeof(Integral) < sizeof(number_type))
    {
        // noexcept if sizeof(UnsignedInteger) < sizeof(number_type)
        // since the underlying vector has enough memory atleast 1 number_type it will never throw!
        mIsNegative = ::zxshady::is_negative(num);
        mNumbers.clear();
        num = math::unsigned_abs(num);
        if ZXSHADY_CONSTEXPR17(sizeof(num) < sizeof(number_type))
        {
            mNumbers.push_back(num);
        }
        else {
            while (num != 0) {
                mNumbers.push_back(num % kMaxDigitsInNumber);
                num /= kMaxDigitsInNumber;
            }
        }

        if (mNumbers.empty())
            mNumbers.push_back(0);
        return *this;
    }

    template<class T>
    T to() const
    {
        static_assert(std::is_integral<T>::value, "T in zxshady::bigint::to<T>() must be an integral type.");

        if ((std::numeric_limits<T>::max)() < *this) {
            throw std::range_error("zxshady::bigint::to<T>() value cannot be represented it is either too big or too small");
        }

        T x{ 0 };

        const auto end = mNumbers.crend();
        for (auto iter = mNumbers.crbegin(); iter != end; ++iter)
            x = *iter + x * kMaxDigitsInNumber;

        if ZXSHADY_CONSTEXPR17(std::is_signed<T>::value)
            if (is_negative())
                x *= -1;
        return x;
    }

    template<class T>
    result<T> non_throwing_to() const noexcept
    {
        static_assert(std::is_integral<T>::value, "T in zxshady::bigint::to<T>() must be an integral type.");

        if ((std::numeric_limits<T>::max)() < *this) {
            return { 0,std::errc::result_out_of_range };
        }

        T x{ 0 };

        const auto end = mNumbers.crend();
        for (auto iter = mNumbers.crbegin(); iter != end; ++iter)
            x = *iter + x * kMaxDigitsInNumber;

        if ZXSHADY_CONSTEXPR17(std::is_signed<T>::value)
            if (is_negative())
                x *= -1;
        return result<T>{ x };
    }

    ZXSHADY_NODISCARD bool is_negative() const noexcept { return mIsNegative; }
    ZXSHADY_NODISCARD bool is_positive() const noexcept { return !is_negative(); }

    /// @brief checks if the bigint is non-zero (faster than *this != 0)
    /// @return true if non-zero, false if zero
    ZXSHADY_NODISCARD explicit operator bool() const noexcept
    {
        return !!(*this);
    }

    /// @brief checks if the bigint is zero (faster than *this == 0)
    /// @return true if zero, false if non-zero
    ZXSHADY_NODISCARD bool operator!() const noexcept
    {
        return mNumbers.size() == 1 && mNumbers[0] == 0;
    }

    /// @brief makes a copy of *this (NOTE to make it positive use bigint::abs(*this))
    /// @return a copy of *this
    ZXSHADY_NODISCARD bigint operator+() const
    {
        return *this;
    };

    /// @brief negates sign
    /// @return negates the sign and returns a copy
    ZXSHADY_NODISCARD bigint operator-() const
    {
        bigint copy = *this;
        copy.flip_sign();
        return copy;
    };

    void swap(bigint& that) & noexcept
    {
        using std::swap;
        swap(mIsNegative, that.mIsNegative);
        swap(mNumbers, that.mNumbers);
    }

    friend void swap(bigint& a,bigint& b) noexcept
    {
        a.swap(b);
    }



    bigint& operator+=(const bigint& rhs)&
    {
        *this = add(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    bigint& operator-=(const bigint& rhs)&
    {
        *this = sub(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    bigint& operator*=(const bigint& rhs)&
    {
        *this = mul(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    bigint& operator/=(const bigint& rhs)&
    {
        *this = div(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    bigint& operator%=(const bigint& rhs)&
    {
        *this = mod(*this, rhs);
        return *this;
    }

    template<class Integer, typename std::enable_if<
        std::is_integral<Integer>::value, int>::type = 0>
    ZXSHADY_NODISCARD friend bool operator==(const bigint& a, Integer b) noexcept
    {
        if (a.is_negative() != zxshady::is_negative(b))
            return false;

        if (a.digit_count() != zxshady::math::digit_count(b))
            return false

        b = math::abs(b);
        
        if ZXSHADY_CONSTEXPR17(sizeof(b) < sizeof(number_type))
        {
            // compare least significant
            return a.mNumbers[0] == b;
        }
        else {
            // a == b
            // i = 0
            // i = 1
            // i = 2
            // a = {0}23
            // b = 123
            std::size_t index = 0;
            const auto size = a.mNumbers.size();
            while (b != 0 && index != size) {
                if (a.mNumbers[index] != b % kMaxDigitsInNumber) {
                    return false;
                }
                else {
                    b /= kMaxDigitsInNumber;
                    index++;
                }
            }
        }
        return true;
    }

    template<class Integer>
    ZXSHADY_NODISCARD friend bool operator==(Integer a, const bigint& b) noexcept
    {
        static_assert(std::is_integral<Integer>::value, "zxshady::operator==(Integer,bigint) Integer must be an integral type.");
        return (b == a);
    }


    ZXSHADY_NODISCARD friend bool operator==(const bigint& a, const bigint& b) noexcept
    {
        return eq(a, b, a.is_negative(), b.is_negative());
    }


    ZXSHADY_NODISCARD friend bool operator<(const bigint& a, const bigint& b) noexcept
    {
        return lt(a, b, a.is_negative(), b.is_negative());
    }


    template<class Integer>
    ZXSHADY_NODISCARD friend bool operator<(const bigint& a, Integer b) noexcept
    {
        static_assert(std::is_integral<Integer>::value, "zxshady::bigint::operator<(bigint,Integer) Integer must an integral type");

        if (a.is_negative() && ::zxshady::is_positive(b))
            return true;
        
        auto acount = a.digit_count();
        auto bcount = zxshady::math::digit_count(b);

        if (acount != bcount)
            return acount < bcount;

        b = math::unsigned_abs(b);

        if ZXSHADY_CONSTEXPR17(sizeof(Integer) < sizeof(number_type))
            return a.mNumbers[0] < b;

        Integer x{ 0 };

        const auto end = a.mNumbers.crend();
        for (auto iter = a.mNumbers.crbegin(); iter != end; ++iter)
            x = *iter + x * kMaxDigitsInNumber;
        return x < b;
    }

    template<class Integer>
    ZXSHADY_NODISCARD friend bool operator<(Integer a, const bigint& b) noexcept
    {
        static_assert(std::is_integral<Integer>::value, "zxshady::bigint::operator<(bigint,Integer) Integer must an integral type");

        if (::zxshady::is_negative(a) && b.is_positive())
            return true;

        const auto acount = math::digit_count(a);
        const auto bcount = b.digit_count();

        if (acount != bcount)
            return acount < bcount;

        a = math::unsigned_abs(a);
        if ZXSHADY_CONSTEXPR17(sizeof(Integer) < sizeof(number_type))
            return a < b.mNumbers[0];

        Integer x{ 0 };

        const auto end = b.mNumbers.crend();
        for (auto iter = b.mNumbers.crbegin(); iter != end; ++iter)
            x = *iter + x * kMaxDigitsInNumber;
        return a < x;
    }


    ZXSHADY_NODISCARD friend bool operator>(const bigint& a, const bigint& b)
    {
        return b < a;
    }

    ZXSHADY_NODISCARD friend bool operator>=(const bigint& a, const bigint& b)
    {
        return !(a < b);
    }
    ZXSHADY_NODISCARD friend bool operator<=(const bigint& a, const bigint& b)
    {
        return !(a > b);
    }

    bigint& operator++()&
    {
        if (is_negative()) {
            mIsNegative = false;
            --*this;
            mIsNegative = true;
        }

        const auto size = mNumbers.size();
        std::size_t index = 0;
    start:

        if (index < size) {
            auto& val = mNumbers[index];

            val += 1;
            if (val >= kMaxDigitsInNumber) {
                val -= kMaxDigitsInNumber;
                index++;
                goto start;
            }
        }
        else {
            mNumbers.push_back(1);
        }
        return *this;
    }


    ZXSHADY_NODISCARD_MSG("zxshady::bigint::operator++(int) post-fix increment incurs memory allocation (via the required copy) and overhead use prefix increment if you don't intend to use the value.")
        bigint operator++(int) &
    {
        bigint copy = *this;
        ++(*this);
        return copy;
    }


    bigint& operator--()&
    {
        if (!*this) {
            mIsNegative = true;
            mNumbers[0] = 1;
            return *this;
        }
        // checking for 0 and is_negative == *this < 1
        if (is_negative()) {
            mIsNegative = false;
            ++*this;
            mIsNegative = true;
            return *this;
        }

        // 120
        //   1
        //  -1
        //   9
        std::size_t index = 0;
    start:
        // [3,0]
        //    1
        // least significant
        auto& back = mNumbers[index]; 

        if (back == 0) {
            back = kMaxDigitsInNumber - 1;
            index++;
            goto start;
        }
        else {
            --back;
        }
        return *this;
    }

    ZXSHADY_NODISCARD_MSG("zxshady::bigint::operator--(int) post-fix increment incurs memory allocation (via the required copy) and overhead use prefix decrement if you don't intend to use the value.")
        bigint operator--(int) &
    {
        auto copy{ *this };
        --(*this);
        return copy;
    }


    bool is_prime() const noexcept;

    bool is_even() const noexcept { return mNumbers.front() % 2 == 0; }
    bool is_odd() const noexcept { return !is_even(); }
    void flip_sign() & noexcept { if (*this) mIsNegative = !mIsNegative; }
    void set_positive() & noexcept { mIsNegative = false; }
    void set_negative() & noexcept { if (*this) mIsNegative = true; }
    void set_sign(bool negative) & noexcept { if (negative) set_negative(); else set_positive(); }

    /// @brief sets value to zero (faster than *this = 0)
    void zero() & noexcept
    {
        mIsNegative = false;
        mNumbers.clear();
        mNumbers.push_back(0);
    }

    /// @brief reverses the number
    /// @note slow function
    void reverse()
    {
        auto str = to_string();
        auto first = str.begin();
        auto last = str.end();

        while (first != last && first != --last)
            std::swap(*(first++), *last);
        *this = bigint{ str };
    }

    bigint& half() & noexcept;
    bigint half() && noexcept
    {
        return this->half(); // calls lvalue overload of half();
    }
    bool is_pow_of_10() const noexcept;
    bool is_pow_of_2() const noexcept;

    std::string to_string() const;

    std::size_t digit_count() const noexcept
    {
        return (mNumbers.size() - 1) * kDigitCountOfMax + math::digit_count(mNumbers.back());
    }

    std::size_t bit_count() const noexcept
    {
        return mNumbers.size() * sizeof(number_type) * CHAR_BIT;
    }

public: /******* FRIENDS AND STATICS ******/


    static bigint pow10(unsigned long long exponent);

    static bigint rand(std::size_t num_digits = 1000);
    static bigint add(const bigint& a, const bigint& b) { return add(a, b, a.is_negative(), b.is_negative()); }
    static bigint sub(const bigint& a, const bigint& b) { return sub(a, b, a.is_negative(), b.is_negative()); }
    static bigint mul(const bigint& a, const bigint& b) { return mul(a, b, a.is_negative(), b.is_negative()); }
    static bigint div(const bigint& a, const bigint& b) { return div(a, b, a.is_negative(), b.is_negative()); }
    static bigint mod(const bigint& a, const bigint& b)
    {
        return b ? sub(a, (mul(div(a, b), b)))
            : throw std::invalid_argument("zxshady::bigint::operator% Modulo by zero");
    }

    /// @brief compares 
    /// @return -1 if a  <  b
    /// @return  0 if a  == b
    /// @return +1 if a  >  b
    template<class T>
    int compare(T that) noexcept
    {
        auto& a = *this;
        auto& b = that;

        if (a < b)
            return -1;
        if (a == b)
            return 0;
        return 1;
    }

    /// @brief compares using absolute value faster than abs(a).compares(abs(b))
    /// @return -1 if a  <  b
    /// @return  0 if a  == b
    /// @return +1 if a  >  b
    int signless_compare(const bigint& that)
    {
        auto& a = *this;
        auto& b = that;
        if (bigint::lt(a, b, false, false))
            return -1;
        if (bigint::eq(a, b, false, false))
            return 0;
        return 1;
    }

private: // internal functions for deciding the sign 
    static bool eq(const bigint& a, const bigint& b, bool a_negative, bool b_negative) noexcept
    {
        return a_negative == b_negative && a.mNumbers == b.mNumbers;
    }

    static bool lt(const bigint& a, const bigint& b, bool a_negative, bool b_negative) noexcept;
    static bigint add(const bigint& a, const bigint& b, bool a_negative, bool b_negative);
    static bigint sub(const bigint& a, const bigint& b, bool a_negative, bool b_negative);
    static bigint mul(const bigint& a, const bigint& b, bool a_negative, bool b_negative);
    static bigint div(const bigint& a, const bigint& b, bool a_negative, bool b_negative);

private:
    void fix() & noexcept
    {
        if (mNumbers.size() == 1 && mNumbers[0] == 0 && is_negative()) {
            mIsNegative = false;
            return;
        }

        if (mNumbers.size() == 0) {
            mNumbers.push_back(0);
            mIsNegative = false;
            return;
        }

        while (mNumbers.size() > 1 && mNumbers.back() == 0)
            mNumbers.pop_back();

        assert(!mNumbers.empty());
    }

private:
    constexpr static number_type kDigitCountOfMax = 9;
    constexpr static number_type kMaxDigitsInNumber = static_cast<number_type>(1e9);
private:
    std::vector<number_type> mNumbers;
    bool mIsNegative;
};

using signed_bigint = zxshady::bigint;

// optimized function overloads
template<class T>
bigint& operator/=(bigint& x,std::integral_constant<T,T(2)>)
{return x.half();}

template<class T>
bigint& operator%=(bigint& x,std::integral_constant<T,T(2)>)
{return x=x.is_even();}

template<class T>
ZXSHADY_NODISCARD bool operator==(const bigint& x,std::integral_constant<T,T(0)>) noexcept
{return !x;}
template<class T>
ZXSHADY_NODISCARD bool operator!=(const bigint& x,std::integral_constant<T,T(0)>) noexcept
{return static_cast<bool>(x);}


template<class T>
ZXSHADY_NODISCARD bool operator==(std::integral_constant<T,T(0)>,const bigint& x) noexcept
{return !x;}
template<class T>
ZXSHADY_NODISCARD bool operator!=(std::integral_constant<T,T(0)>,const bigint& x) noexcept
{return static_cast<bool>(x);}


template<class T> ZXSHADY_NODISCARD bool operator< (const bigint& x,std::integral_constant<T,T(0)>) noexcept { return x.is_negative();}
template<class T> ZXSHADY_NODISCARD bool operator> (const bigint& x,std::integral_constant<T,T(0)>) noexcept { return x.is_positive() && x;}
template<class T> ZXSHADY_NODISCARD bool operator<=(const bigint& x,std::integral_constant<T,T(0)>) noexcept { return x.is_negative() || !x;}
template<class T> ZXSHADY_NODISCARD bool operator>=(const bigint& x,std::integral_constant<T,T(0)>) noexcept { return x.is_positive() || !x;}

template<class T> ZXSHADY_NODISCARD bool operator> (std::integral_constant<T,T(0)>,const bigint& x) noexcept { return x.is_negative();}
template<class T> ZXSHADY_NODISCARD bool operator< (std::integral_constant<T,T(0)>,const bigint& x) noexcept { return x.is_positive() && x;}
template<class T> ZXSHADY_NODISCARD bool operator<=(std::integral_constant<T,T(0)>,const bigint& x) noexcept { return x.is_positive() || !x;}
template<class T> ZXSHADY_NODISCARD bool operator>=(std::integral_constant<T,T(0)>,const bigint& x) noexcept { return x.is_negative() || !x;}


ZXSHADY_NODISCARD inline bigint operator+(bigint lhs, const bigint& rhs) { return lhs += rhs; }
ZXSHADY_NODISCARD inline bigint operator-(bigint lhs, const bigint& rhs) { return lhs -= rhs; }
ZXSHADY_NODISCARD inline bigint operator*(bigint lhs, const bigint& rhs) { return lhs *= rhs; }
ZXSHADY_NODISCARD inline bigint operator/(bigint lhs, const bigint& rhs) { return lhs /= rhs; }
ZXSHADY_NODISCARD inline bigint operator%(bigint lhs, const bigint& rhs) { return lhs %= rhs; }

//using unsigned_bigint = zxshady::bigint;
ZXSHADY_NODISCARD inline bool operator==(const std::string& a, const bigint& b) noexcept
{return (bigint{ a } == b);}
ZXSHADY_NODISCARD inline bool operator==(const bigint& a,const std::string& b) noexcept
{return (a == bigint{ b });}

ZXSHADY_NODISCARD inline bool operator==(const char* a, const bigint& b) noexcept
{return (bigint{ a } == b);}
ZXSHADY_NODISCARD inline bool operator==(const bigint& a,const char* b) noexcept
{return (a == bigint{ b });}

#ifndef ZXSHADY_CPP20 // no need in C++20 :) thank you C++ for being a decent language

ZXSHADY_NODISCARD inline bool operator!=(const bigint& a, const bigint& b) noexcept{return !(a==b);}
template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 > ZXSHADY_NODISCARD bool operator!=(Integer a, const bigint& b) noexcept { return !(a == b); }
template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 > ZXSHADY_NODISCARD bool operator!=(const bigint& a, Integer b) noexcept { return !(a == b); }

#endif // !defined(ZXSHADY_CPP20)



template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>(const bigint& a,Integer b) noexcept
{
    return b < a;
}

template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator<=(const bigint& a,Integer b) noexcept
{
    return !(a > b);
}

template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>=(const bigint& a,Integer b) noexcept
{
    return !(a < b);
}

template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>(Integer a,const bigint& b) noexcept
{
    return b < a;
}

template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator<=(Integer a,const bigint& b) noexcept
{
    return !(a > b);
}

template<class Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>=(Integer a,const bigint& b) noexcept
{
    return !(a < b);
}

inline bigint abs(bigint x)
{
    x.set_positive();
    return x;
}

inline bigint fac(bigint x)
{
    // assumes no one is going to try factorial of number bigger than long max
    // if you REALLY really want it then just make
    bigint y = x;
    
    const auto end = x.template to<std::size_t>();
    // 3
    // 3 * (2-1)

    for (std::size_t i = 2; i <= end; ++i) {
        --y;
        x *= y;
    }
    return x;
}
unsigned long long log2(bigint x);
unsigned long long log10(const bigint& x);
unsigned long long log(const bigint& x, unsigned long long base);
bigint pow(bigint base, unsigned long long exponent);
bigint sqrt(const bigint& x);
bigint gcd(bigint a, bigint b);
bigint lcm(const bigint& a, const bigint& b);


std::ostream& operator<<(std::ostream& ostream, const bigint& bignum);
std::istream& operator>>(std::istream& istream, bigint& bignum);

    inline namespace literals {

        inline namespace const_literals {
        // from post 
        // https://codereview.stackexchange.com/questions/50910/user-defined-literal-for-stdintegral-constant
            namespace details {
            template<typename T, typename U>
            constexpr T pow_helper(T acc, T value, U times)
            {
                return (times > 1) ?
                    pow_helper(acc*value, value, times-1) :
                    acc;
            }

            template<typename T, typename U>
            constexpr T pow(T value, U exponent)
            {
                return (exponent == 0) ? 1 :
                    (exponent > 0) ? pow_helper(value, value, exponent) :
                    1 / pow_helper(value, value, -exponent);
            }

            template<typename Integral, char C, char... Digits>
            struct parse
            {
                static constexpr Integral value =
                    static_cast<Integral>(C-'0') * pow(10u, sizeof...(Digits))
                    + parse<Integral, Digits...>::value;
            };

            template<typename Integral, char C>
            struct parse<Integral, C>
            {
                static_assert(C >= '0' && C <= '9',
                    "only characters in range 0..9 are accepted");

                static constexpr Integral value = C - '0';
            };

            }

            // User defined literal for std::integral_constant
            template<char... Digits> constexpr ::std::integral_constant<int, details::parse<int, Digits...>::value> 
            operator""_c() noexcept {return {};}

            template<char... Digits> constexpr ::std::integral_constant<unsigned int, details::parse<unsigned int, Digits...>::value> 
            operator""_uc() noexcept {return {};}

            template<char... Digits> constexpr ::std::integral_constant<unsigned long, details::parse<unsigned long, Digits...>::value> 
            operator""_ulc() noexcept {return {};}

            template<char... Digits> constexpr ::std::integral_constant<unsigned long long, details::parse<unsigned long long, Digits...>::value> 
            operator""_ullc() noexcept {return {};}

            template<char... Digits> constexpr ::std::integral_constant<long, details::parse<long, Digits...>::value> 
            operator""_lc() noexcept {return {};}

            template<char... Digits> constexpr ::std::integral_constant<long long, details::parse<long long, Digits...>::value> 
            operator""_llc() noexcept {return {};}


        }
        inline namespace bigint_literals {
        
        inline ::zxshady::bigint operator""_big(unsigned long long x)
        {
            return bigint{ x };
        }

        inline ::zxshady::bigint operator""_big(const char* str,std::size_t len)
        {
            return bigint{ str,str + len };
        }

        }

    }


#include <stdexcept>
#include <cctype>
#include <cmath>
#include <iostream>
#include <random>
#include <limits>

#include "bigint.hpp"

    using namespace zxshady;

    inline bigint::bigint(const char* begin,const char* end)
    {
        if (*begin == '-') {
            mIsNegative = true;
            ++begin;
        }
        else {
            mIsNegative = false;
        }

        // "01208021380"
        while(begin != end && *begin == '0')
            ++begin;

        std::reverse_iterator<const char*> rbegin{ end };
        std::reverse_iterator<const char*> rend{ begin };

        auto it = rbegin;
        while (it != rend) {
            number_type num = 0;
            for (std::size_t i = 0; i < kDigitCountOfMax && it != rend; ++i, ++it) {
                if (std::isdigit(*it)) {
                    //1234567890
                    num += static_cast<number_type>(math::pow10(i) * (*it - '0'));
                } else {
                    throw std::invalid_argument("zxshady::bigint(const char* begin,const char* end) invalid string representation at position " + std::to_string(i) + " character was " + std::string(1,*it));
                }
            }
            mNumbers.push_back(num);
        }

        fix();
        if (mNumbers.empty())
            *this = bigint();
    }
    inline bigint zxshady::sqrt(const bigint& x)
    {
        if (x.is_negative())
            throw std::invalid_argument("zxshady::sqrt(const bigint& x) x must not be negative");

        // 1 4 9 16 25 36
        if (!x)
            return bigint{ 0 };

        if (x < 4)
            return 1;
        if (x < 9)
            return 2;
        if (x < 16)
            return 3;
        if (x < 25)
            return 4;
        if (x < 36)
            return 5;
        if (x < 49)
            return 6;
        if (x < 64)
            return 7;


        bigint n = x;
        bigint x0 = 1;
    start:
        const bigint x1 = ((n / x0) + x0) / 2;
        if (x0 == x1 || x0 == (x1 - 1)) {
            return x0;
        }
        x0 = x1;
        goto start;
        return x0;
    }
    inline bool bigint::is_pow_of_10() const noexcept
    {
        const auto end = mNumbers.cend() - 1;
        auto iter = mNumbers.cbegin();
        for(;iter != end;++iter)
            if (*iter != 0)
                return false;


        switch (mNumbers.back()) {
            case static_cast<number_type>(1):
            case static_cast<number_type>(10):
            case static_cast<number_type>(100):
            case static_cast<number_type>(1000):
            case static_cast<number_type>(10000):
            case static_cast<number_type>(100000):
            case static_cast<number_type>(1000000):
            case static_cast<number_type>(10000000):
            case static_cast<number_type>(100000000):
            case static_cast<number_type>(1000000000):
                return true;
        }
        return false;

    }
    inline bool bigint::is_pow_of_2() const noexcept
    {
        if (!*this) // == 0
            return true;

        auto x = *this;

        while (x.is_even())
            x.half();
        return x == (unsigned char)1;
    }
    inline std::string bigint::to_string() const
    {
        std::string ret = is_negative()
            ? "-"
            : "";
        const auto begin = mNumbers.crbegin();
        const auto end = mNumbers.crend();
        auto iter = begin;
        if (iter == end) return ret;
        ret += std::to_string(*iter);
        ++iter;
        // Print the digits in reverse order
        // 135790, 753135, 390135, 13051, 91035, 610009, 102345
        // [0,10]
        // 10 ' 000000
        for (; iter != end; ++iter) {
            auto val = *iter;
            auto cnt = zxshady::math::digit_count(val);
            //printf("%d / %d", (int)digitcount(val), (int)kDigitCountOfMax);
            auto difference = bigint::kDigitCountOfMax - cnt;

            static constexpr const char* zeroes[] = {
                "",
                "0",
                "00",
                "000",
                "0000",
                "00000",
                "000000",
                "0000000",
                "00000000",
                "000000000",
                "0000000000",
                "00000000000",
                "000000000000",
                "0000000000000",
                "00000000000000",
                "000000000000000",
                "0000000000000000",
                "00000000000000000",
                "000000000000000000",
                "0000000000000000000",
                "00000000000000000000",
                "000000000000000000000",
                "0000000000000000000000",
                "00000000000000000000000",
                "000000000000000000000000"
            };
            assert(difference < sizeof(zeroes) / sizeof(zeroes[0]));
            ret += zeroes[difference];
            ret += std::to_string(*iter);
        }
        return ret;
    }
    inline bigint bigint::add(const bigint& a, const bigint& b, bool a_negative, bool b_negative) {

        if (!a_negative && b_negative) {
            // a + -b = a - b 
            return sub(a, b, false, false);
        }

        if (a_negative && !b_negative) {
            // -a + b = b - a 
            return sub(b, a, false, false);
        }

        if (a_negative && b_negative) {
            // -a + -b = -(b + a)
            auto ret = add(a, b, false, false);
            ret.set_negative();
            return ret;
        }

        bigint ret{ noinit_t{} };
        number_type carry = 0;

        const auto asize = a.mNumbers.size();
        const auto bsize = b.mNumbers.size();
        const auto max = std::max(asize, bsize);

        for (std::size_t i = 0; i < max; ++i) {
            number_type sum = carry;
            if (i < asize)
                sum += a.mNumbers[i];
            if (i < bsize)
                sum += b.mNumbers[i];

            ret.mNumbers.push_back(sum % kMaxDigitsInNumber);
            carry = sum / kMaxDigitsInNumber;
        }
        if (carry > 0)
            ret.mNumbers.push_back(carry);

        ret.fix();
        return ret;
    }
    inline bigint bigint::sub(const bigint& a, const bigint& b, bool a_negative, bool b_negative)
    {
        if (!a_negative && b_negative) {
            // a - -b == a + b
            return add(a, b, false, false);
        }

        if (a_negative && !b_negative) {
            // -a - b == -(a+b)
            auto ret = add(a, b, false, false);
            ret.set_negative();
            return ret;
        }

        if (a_negative && b_negative) {
            // -a - -b == (b - a)
            auto ret = sub(b, a, false, false);
            ret.set_negative();
            return ret;

        }



        if (a < b) {
            auto ret = b - a;
            ret.flip_sign();
            return ret;
        }

        //   2132
        // -  343
        // 
        //  132
        //  343
        // -211 (+1000)
        // +899
        //  1 - 0

        bigint ret{ zxshady::noinit_t{} };
        std::int64_t carry = 0;
        const auto asize = a.mNumbers.size();
        const auto bsize = b.mNumbers.size();
        const auto max = (std::max)(asize, bsize);
        for (std::size_t i = 0; i < max; i++) {
            std::int64_t diff = carry;

            if (i < asize)
                diff += static_cast<std::int64_t>(a.mNumbers[i]);

            if (i < bsize) {
                diff -= static_cast<std::int64_t>(b.mNumbers[i]);
            }

            if (diff < 0) {
                diff += kMaxDigitsInNumber;
                carry = -1;
            }
            else {
                carry = 0;
            }
            ret.mNumbers.push_back(static_cast<number_type>(diff % kMaxDigitsInNumber));
        }
        ret.fix();
        return ret;
    }
    inline bigint bigint::mul(const bigint& a, const bigint& b, bool a_negative, bool b_negative) 
    {
        if (!a || !b) // using operator! to test for == 0 since it is hardcoded it will be faster!
            return bigint{};

        bigint ret{ noinit_t{} };
        const auto asize = a.mNumbers.size();
        const auto bsize = b.mNumbers.size();

        ret.mNumbers.resize(asize + bsize + 1);

        for (std::size_t i = 0; i < asize; i++) {
            std::uint64_t carry = 0;
            for (std::size_t j = 0; j < bsize; j++) {
                const std::uint64_t product = static_cast<std::uint64_t>(a.mNumbers[i]) * static_cast<std::uint64_t>(b.mNumbers[j]) +
                    carry + static_cast<std::uint64_t>(ret.mNumbers[i + j]);
                ret.mNumbers[i + j] = static_cast<number_type>(product % kMaxDigitsInNumber);
                carry = product / kMaxDigitsInNumber;
            }
            assert(carry < static_cast<std::uint64_t>(kMaxDigitsInNumber));
            ret.mNumbers[i + bsize] += static_cast<number_type>(carry);
        }

        // remove leading zeroes
        ret.mIsNegative = a_negative != b_negative;
        ret.fix();

        return ret;
    }
    inline bigint bigint::div(const bigint& a, const bigint& b, bool a_negative, bool b_negative)
    {
        if (!b)
            throw std::invalid_argument("zxshady::bigint::operator/ Division by zero.");

        // if the abs(a) is less than abs(b), the quotient is 0
        if (bigint::lt(a, b, false, false)) {
            return bigint{};
        }


        bigint current{};  // current number in process of division
        bigint ret{ noinit_t{} };
        ret.mNumbers.reserve(a.mNumbers.size());
        auto index = ret.mNumbers.size() - 1;
        const auto end = a.mNumbers.crend();
        for (auto iter = a.mNumbers.crbegin(); iter != end; ++iter) {
            current *= kMaxDigitsInNumber;
            current += *iter;
            std::uint32_t start = 0;
            std::uint32_t end = kMaxDigitsInNumber;
            while (start <= end) {
                std::uint32_t mid = (start + end) / 2;
                if (mid * b <= current)
                    start = mid + 1;
                else
                    end = mid - 1;
            }
            ret.mNumbers.insert(ret.mNumbers.begin(), end);
            current -= end * b;
        }

        ret.mIsNegative = a_negative != b_negative;
        ret.fix();
        return ret;
    }
    inline bool bigint::lt(const bigint& a, const bigint& b,bool a_negative,bool b_negative) noexcept
    {
        // if not same sign...
        if (a_negative != b_negative)
            return a_negative;

        // if not same size
        const auto asize = a.mNumbers.size();
        const auto bsize = b.mNumbers.size();

        if (asize != bsize)
            return (asize < bsize) != a_negative;

        // Compare digits starting from the most significant
        const auto end = a.mNumbers.crend();
        auto it1 = a.mNumbers.crbegin();
        auto it2 = b.mNumbers.crbegin();


        // void() to prevent overloading of comma operator :P
        for (; it1 != end; ++it1, void(), ++it2)
            if (*it1 != *it2)
                return (*it1 < *it2) != a_negative;

        return false;
    }
    inline bigint& bigint::half() & noexcept
    {
        if (this->is_positive() && *this <= 2) {
            auto& first = mNumbers[0];
            first = first == 2;
            return *this;
        }

        bigint current{};  // current number in process of division
        std::vector<number_type> res;
        res.reserve(mNumbers.size());

        const auto end = mNumbers.crend();
        // 513 
        //   2
        // 256

        for (auto iter = mNumbers.crbegin(); iter != end; ++iter) {
            current *= kMaxDigitsInNumber;

            current += *iter;
            // 3
            number_type start = 0;
            number_type end = kMaxDigitsInNumber;
            while (start <= end) {
                number_type mid = (start + end) / 2;
                // 5
                if (mid * 2 <= current)
                    start = mid + 1;
                else
                    end = mid - 1;
                //  4
            }
            res.insert(res.begin(), end % kMaxDigitsInNumber);
            current -= end * 2;
        }

        this->mNumbers = std::move(res);
        this->fix();
        return *this;
    }
    inline bigint bigint::rand(std::size_t num_digits /* = 1000 */)
    {
        if(num_digits == 0)
            throw std::invalid_argument("zxshady::bigint::rand(size_t num_digits) num_digits must be non-zero");

        bigint ret{ noinit_t{} };
        std::size_t total = 0;
        while (total != num_digits) {
            // 21
            auto rand = static_cast<number_type>(std::rand());
            while(rand <= kMaxDigitsInNumber)
                rand *= static_cast<number_type>(std::rand());

            // 13049831058
            rand %= kMaxDigitsInNumber;
            if(rand == 0)
                rand = 1;

            if(rand % 25 == 0){
                ret.flip_sign();
            }
            total += kDigitCountOfMax;
            if (total > num_digits) {
                total -= kDigitCountOfMax;
                total += zxshady::math::digit_count(rand);
            }
            ret.mNumbers.push_back(rand);

            while(total > num_digits){
                auto& back = ret.mNumbers.back();
                back /=10;
                --total;
            }
        }
        ret.fix();
        assert(ret.digit_count() == num_digits);
        return ret;
    }
    inline unsigned long long zxshady::log2(bigint x)
    {
        if(!x)
            throw std::invalid_argument("zxshady::bigint::log2(bigint x) input cannot be 0");

        if(x.is_negative())
            throw std::invalid_argument("zxshady::bigint::log2(bigint x) input cannot be negative");

        unsigned long long ret = 0;
        while(x) {
            ++ret;
            x.half();
        }
        return ret - 1;

    }
    inline unsigned long long zxshady::log10(const bigint& x)
    {
        if(!x)
            throw std::invalid_argument("zxshady::bigint::log10(bigint x) input cannot be 0");
        if(x.is_negative())
            throw std::invalid_argument("zxshady::bigint::log10(bigint x) input cannot be negative");

        return x.digit_count() - 1;
    }
    inline unsigned long long zxshady::log(const bigint& x,unsigned long long base )
    {
        if(!x)
            throw std::invalid_argument("zxshady::bigint::log(bigint x,unsigned long long base) x cannot be 0");

        if(x.is_negative())
            throw std::invalid_argument("zxshady::bigint::log(bigint x,unsigned long long base) x cannot be negative");

        if(base <= 1)
            throw std::invalid_argument("zxshady::bigint::log(bigint x,unsigned long long base) base cannot be less than or equal to 1");


        return static_cast<unsigned long long>(static_cast<double>(log10(x)) / std::log10(base));
    }
    inline bigint zxshady::pow(bigint base, unsigned long long exponent)
    {
        if (!base)
            return bigint{};

        if (base == 10) // short cut for faster execution
            return bigint::pow10(exponent);

        if (base == 1)
            return bigint{ 1 };

        if (exponent == 0)
            return bigint{ 1 };

        if (exponent == 1)
            return base;

        const bool was_negative = exponent % 2 == 1&& base.is_negative();
        base.set_positive();
        const bigint mult = base;
        while (exponent > 1) {
            base *= mult;
            --exponent;
        }

        base.set_sign(was_negative);
        return base;
    }
    inline bigint bigint::pow10(unsigned long long exponent)
    {
        if (exponent == 0)
            return bigint{ 1 };
        bigint ret;
        ret.mNumbers.resize(exponent / kDigitCountOfMax + 1);
        ret.mNumbers[exponent / kDigitCountOfMax] = static_cast<number_type>(math::pow10(exponent % kDigitCountOfMax));
        return ret;
    }
    inline bool zxshady::bigint::is_prime() const noexcept
    {
        if (*this == (unsigned char)2)
            return true;

        if (is_negative() || is_even() || !*this || !(mod(*this, 3)))
            return false;


        bigint range = sqrt(*this);
        const bigint six = (char)6;
        for (bigint i = 5; i <= range; i += six)
            if (!(*this % i) || !(*this % (i + 2)))
                return false;
        return true;
    }
    inline bigint zxshady::gcd(bigint a, bigint b)
    {
        if (!b) // using operator! for == 0 becuase it is faster since it is hardcoded!
            return abs(a);    // gcd(a, 0) = |a|
        if (!a) // using operator! for == 0 becuase it is faster since it is hardcoded!
            return abs(b);    // gcd(0, a) = |a|

        a.set_positive();
        b.set_positive();

        bigint remainder = b;
        while (remainder) {
            remainder = a % b;
            a = b;  
            b = remainder; 
        }
        return a;
    }
    inline bigint zxshady::lcm(const bigint& a, const bigint& b)
    {
        if (!a || !b ) // using negation operator! to check for == 0 since it is hardcoded it is faster!
            return bigint{};
        return abs(a * b) / gcd(a, b);

    }
    inline std::ostream& zxshady::operator<<(std::ostream& ostream, const zxshady::bigint& bigint)
    {
        return ostream << bigint.to_string();
    }
    inline std::istream& zxshady::operator>>(std::istream& istream, zxshady::bigint& bigint)
    {
        std::string s;
        istream >> s;
        if (istream.good())
            bigint = s;
        else
            bigint.zero();
        return istream;
    }


}


#endif // !ZXSHADY_BIGINT_HPP



