#ifndef ZXSHADY_BIGINT_HPP
#define ZXSHADY_BIGINT_HPP

#include <iosfwd>

#include <vector>
#include <string>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <system_error>

#ifdef ZXSHADY_CPP20

#include <compare>

#endif

#include "../../zxshady/cpp_version.hpp"
#include "../../zxshady/macros.hpp"
#include "../../zxshady/math.hpp"
#include "iterator.hpp"
#include "reference.hpp"

namespace zxshady {

template<typename>
struct always_false : public std::false_type {};

template<typename T>
struct double_width { using type = T; };

template<> struct double_width<std::uint8_t>  { using type = std::uint16_t; };
template<> struct double_width<std::uint16_t> { using type = std::uint32_t; };
template<> struct double_width<std::uint32_t> { using type = std::uint64_t; };

template<> struct double_width<std::int8_t>  { using type = std::int16_t; };
template<> struct double_width<std::int16_t> { using type = std::int32_t; };
template<> struct double_width<std::int32_t> { using type = std::int64_t; };

template<typename T>
struct half_width { using type = T; };

template<> struct half_width<std::uint16_t> { using type = std::uint8_t; };
template<> struct half_width<std::uint32_t> { using type = std::uint16_t; };
template<> struct half_width<std::uint64_t> { using type = std::uint32_t; };

template<> struct half_width<std::int16_t> { using type = std::int8_t; };
template<> struct half_width<std::int32_t> { using type = std::int16_t; };
template<> struct half_width<std::int64_t> { using type = std::int32_t; };

class bigint_format_error : public std::runtime_error {
public:
    using base = std::runtime_error;
    explicit bigint_format_error(const std::string& msg) : base(msg.c_str()) {}
    explicit bigint_format_error(const char* msg) : base(msg) {}
};

// copied from cpp.reference
// https://en.cppreference.com/w/cpp/utility/unreachable
[[noreturn]] inline void unreachable()
{
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}
template<typename T>
struct remove_cvref : public std::remove_cv<typename std::remove_reference<T>::type> {};


#ifdef __cpp_alias_templates

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

#endif

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
private:
    using number_type = std::uint32_t;
    using DoubleWidthType = typename double_width<number_type>::type;
    constexpr static number_type kMaxDigitsInNumber = static_cast<number_type>(1e9);
    constexpr static auto kDigitCountOfMaxInHex = ::zxshady::math::constexpr_log(kMaxDigitsInNumber, 16);
    constexpr static auto kDigitCountOfMaxInBinary = ::zxshady::math::constexpr_log(kMaxDigitsInNumber, 2);
    constexpr static auto kDigitCountOfMax = ::zxshady::math::constexpr_log(kMaxDigitsInNumber, 10);
public:

    constexpr static char default_seperator = '\'';
    constexpr static char no_seperator      = '\0';

    enum base {
        dec = 10,
        hex = 16,
        bin = 2,
        oct = 8
    };

    using iterator = ::zxshady::details::bigint::iterator<number_type, kDigitCountOfMax>;
    using const_iterator = ::zxshady::details::bigint::iterator<const number_type, kDigitCountOfMax>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    using reference = ::zxshady::details::bigint::reference<number_type>;
    using const_reference = ::zxshady::details::bigint::reference<const number_type>;

private:
    //using storage_type = std::vector<number_type>;

    using storage_type = std::basic_string<number_type>;
public:
    // @brief a constructor that initializes the bigint to 0
    bigint()
        : mNumbers(1, 0)
        , mIsNegative(false)
    {
    }


    bigint(const bigint&) = default;
    bigint(bigint&&) noexcept = default;
    bigint& operator=(const bigint&) & = default;
    bigint& operator=(bigint&&) &noexcept = default;
    ~bigint() noexcept = default;

    template<typename Integer, typename std::enable_if<
        std::is_integral<Integer>::value, int>::type = 0>
    bigint(Integer num);


    bigint(std::nullptr_t) = delete;

    template<typename InputIter, typename std::enable_if<
        !std::is_integral<InputIter>::value, int>::type = 0>
    bigint(InputIter begin, InputIter end,bigint::base base,char seperator = default_seperator);

    template<typename InputIter, typename std::enable_if<
        !std::is_integral<InputIter>::value, int>::type = 0>
    bigint(InputIter begin, InputIter end,char seperator = default_seperator);

    template<typename Char,typename std::enable_if<std::is_same<char,Char>::value,int>::type = 0>
    bigint(const char*/*string*/, Char /*seperator*/)
    {
        static_assert(zxshady::always_false<Char>::value,
            "bigint(const char* string,char size) overload is deleted becuase"
            "\nthe char you entered will count as the size instead of a seperator"
            "\nif you want to use the char as the size static_cast it into std::size_t"
            "\nif you want it as the seperator you have to use the iterator constructor.");
    }

    bigint(const char* str, std::size_t size) : bigint(str, str + size) {}
    explicit bigint(const std::string& str) : bigint(str.c_str(), str.c_str() + str.size()) {}
    explicit bigint(const char* str) : bigint(str, str + std::strlen(str)) {}
#ifdef __cpp_lib_string_view

    explicit bigint(std::string_view str) : bigint(str.begin(), str.end()) {};

#endif // defined(__cpp_lib_string_view)


    bigint& operator=(const char* s) &
    {
        return *this = bigint{ s };
    }

    bigint& operator=(const std::string& s) &
    {
        return *this = bigint{ s };
    }

    template<typename Integral, typename std::enable_if<
        std::is_integral<Integral>::value, int>::type = 0>
    bigint& operator=(Integral num) & noexcept(sizeof(Integral) < sizeof(number_type))
    {
        // noexcept if sizeof(UnsignedInteger) < sizeof(number_type)
        // since the underlying vector has enough memory atleast 1 number_type it will never throw!
        mIsNegative = ::zxshady::is_negative(num);
        mNumbers.clear();
        auto unsigned_num = math::unsigned_abs(num);
        if ZXSHADY_CONSTEXPR17(sizeof(num) < sizeof(number_type))
        {
            mNumbers.push_back(unsigned_num);
        }
        else {
            while (unsigned_num != 0) {
                mNumbers.push_back(unsigned_num % kMaxDigitsInNumber);
                unsigned_num /= kMaxDigitsInNumber;
            }
        }

        if (mNumbers.empty())
            mNumbers.push_back(0);
        return *this;
    }

    template<typename T>
    ZXSHADY_NODISCARD T to() const
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

    template<typename T>
    ZXSHADY_NODISCARD result<T> non_throwing_to() const noexcept
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
    ZXSHADY_NODISCARD bool is_positive() const noexcept { return !mIsNegative && *this; }

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

    void swap(bigint& that) & noexcept
    {
        using std::swap;
        swap(mIsNegative, that.mIsNegative);
        swap(mNumbers, that.mNumbers);
    }

    bigint& operator+=(const bigint& rhs) &
    {
        bigint::add_compound(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    template<typename Integer>
    bigint& operator+=(Integer rhs) &
    {
        bigint::add_compound(*this, rhs, this->is_negative(), ::zxshady::is_negative(rhs));
        return *this;
    }

    bigint& operator-=(const bigint& rhs) &
    {
        bigint::sub_compound(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    bigint& operator-=(Integer rhs) &
    {
        bigint::sub_compound(*this, rhs, this->is_negative(), ::zxshady::is_negative(rhs));
        return *this;
    }

    bigint& operator*=(const bigint& rhs) &
    {
        *this = mul(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    template<typename Integer>
    bigint& operator*=(Integer rhs) &
    {
        *this = mul(*this, rhs, this->is_negative(), ::zxshady::is_negative(rhs));
        return *this;
    }

    bigint& operator/=(const bigint& rhs) &
    {
        *this = div(*this, rhs, this->is_negative(), rhs.is_negative());
        return *this;
    }

    template<typename Integer>
    bigint& operator/=(Integer rhs) &
    {
        *this = div(*this, rhs, this->is_negative(), ::zxshady::is_negative(rhs));
        return *this;
    }

    bigint& operator%=(const bigint& rhs) &
    {
        const auto& ref = bigint::div(*this, rhs, this->is_negative(), rhs.is_negative());
        *this -= bigint::mul(ref, rhs, ref.is_negative(), rhs.is_negative());
        return *this;
    }

    template<typename Integer, typename std::enable_if<
        std::is_integral<Integer>::value, int>::type = 0>
    ZXSHADY_NODISCARD friend bool operator==(const bigint& a, Integer b) noexcept
    {
        if (a.is_negative() != zxshady::is_negative(b))
            return false;

        auto unsigned_b = zxshady::math::unsigned_abs(b);

        if ZXSHADY_CONSTEXPR17(sizeof(b) < sizeof(number_type))
        {
            // compare least significant
            return a.mNumbers.size() == 1 && a.mNumbers[0] == unsigned_b;
        }
        else {
            std::size_t index = 0;
            const auto size = a.mNumbers.size();
            while (unsigned_b != 0 && index != size) {
                if (a.mNumbers[index] != unsigned_b % kMaxDigitsInNumber) {
                    return false;
                }
                else {
                    unsigned_b /= kMaxDigitsInNumber;
                    index++;
                }
            }
            return true;
        }
    }


    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    ZXSHADY_NODISCARD friend bool operator==(Integer a, const bigint& b) noexcept
    {
        return b == a;
    }


    ZXSHADY_NODISCARD friend bool operator==(const bigint& a, const bigint& b) noexcept
    {
        return eq(a, b, a.is_negative(), b.is_negative());
    }


    ZXSHADY_NODISCARD friend bool operator<(const bigint& a, const bigint& b) noexcept
    {
        return lt(a, b, a.is_negative(), b.is_negative());
    }


    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    ZXSHADY_NODISCARD friend bool operator<(const bigint& a, Integer b) noexcept
    {
        if (a.is_negative() && ::zxshady::is_positive(b))
            return true;

        auto acount = a.digit_count();
        auto bcount = zxshady::math::digit_count(b);

        if (acount != bcount)
            return (acount < bcount) != a.is_negative();

        auto unsigned_b = math::unsigned_abs(b);

        if ZXSHADY_CONSTEXPR17(sizeof(Integer) < sizeof(number_type))
            return a.mNumbers[0] < unsigned_b;
        else {
            decltype(unsigned_b) x{ 0 };

            const auto end = a.mNumbers.crend();
            for (auto iter = a.mNumbers.crbegin(); iter != end; ++iter)
                x = static_cast<decltype(unsigned_b)>(*iter + x * kMaxDigitsInNumber);
            return x < unsigned_b;
        }
    }

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    ZXSHADY_NODISCARD friend bool operator<(Integer a, const bigint& b) noexcept
    {
        if (::zxshady::is_negative(a) && b.is_positive())
            return true;

        const auto acount = math::digit_count(a);
        const auto bcount = b.digit_count();

        if (acount != bcount)
            return (acount < bcount) != ::zxshady::is_negative(a);

        auto unsigned_a = math::unsigned_abs(a);
        if ZXSHADY_CONSTEXPR17(sizeof(Integer) < sizeof(number_type))
            return unsigned_a < b.mNumbers[0];
        else {
            decltype(unsigned_a) x{ 0 };

            const auto end = b.mNumbers.crend();
            for (auto iter = b.mNumbers.crbegin(); iter != end; ++iter)
                x = *iter + x * kMaxDigitsInNumber;
            return unsigned_a < x;
        }
    }

    bigint& operator++() &
    {
        if (is_negative()) {
            set_positive();
            --(*this);
            set_negative();
            return *this;
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


    

    bigint& operator--() &
    {
        if (!*this) {
            mIsNegative = true;
            mNumbers[0] = 1;
            return *this;
        }
        // checking for 0 and is_negative == *this < 1
        if (is_negative()) {
            set_positive();
            ++(*this);
            set_negative();
            return *this;
        }

        std::size_t index = 0;
    start:
        auto& back = mNumbers[index];

        if (back == 0) {
            back = kMaxDigitsInNumber - 1;
            index++;
            goto start;
        }

        --back;
        return *this;
    }

    ZXSHADY_NODISCARD_MSG("zxshady::bigint::operator++(int) post-fix increment incurs memory allocation (via the required copy) and overhead use prefix increment if you don't intend to use the value.")
    bigint operator++(int) &
    {
        bigint copy = *this;
        ++(*this);
        return copy;
    }
    ZXSHADY_NODISCARD_MSG("zxshady::bigint::operator--(int) post-fix decrement incurs memory allocation (via the required copy) and overhead use prefix decrement if you don't intend to use the value.")
    bigint operator--(int) &
    {
        bigint copy = *this;
        --(*this);
        return copy;
    }

    ZXSHADY_NODISCARD reference at(std::size_t index)
    {
        return index < digit_count() ? operator[](index)
                                     : throw std::out_of_range("zxshady::bigint::at(std::size_t index) out of range");
    }

    ZXSHADY_NODISCARD const_reference at(std::size_t index) const
    {

        return index < digit_count() ? operator[](index)
                                     : throw std::out_of_range("zxshady::bigint::at(std::size_t index) out of range");
    }


    ZXSHADY_NODISCARD reference operator[](std::size_t index) noexcept
    {
        assert(index < digit_count());
        return reference(&mNumbers[index / kDigitCountOfMax],index % kDigitCountOfMax);
    }

    ZXSHADY_NODISCARD const_reference operator[](std::size_t index) const noexcept
    {
        assert(index < digit_count());
        return const_reference(const_cast<number_type*>(&mNumbers[index / kDigitCountOfMax]), index % kDigitCountOfMax);
    }

    /* utility accessors */
    ZXSHADY_NODISCARD reference       front()       noexcept { return reference(&mNumbers.front(),0); };
    ZXSHADY_NODISCARD reference       back()        noexcept { return reference(&mNumbers.back(), static_cast<int>(math::digit_count(mNumbers.back())) - 1); };
    ZXSHADY_NODISCARD const_reference front() const noexcept { return const_reference(&mNumbers.front(),0); };
    ZXSHADY_NODISCARD const_reference back()  const noexcept { return const_reference(&mNumbers.back(), static_cast<int>(math::digit_count(mNumbers.back()) - 1)); };

    /* iterators */
    ZXSHADY_NODISCARD iterator               begin()         noexcept { return iterator(&mNumbers[0], 0); }
    ZXSHADY_NODISCARD iterator               end()           noexcept { return iterator(&mNumbers.back(), static_cast<int>(math::digit_count(mNumbers.back()))); }
    ZXSHADY_NODISCARD const_iterator         begin()   const noexcept { return cbegin(); }
    ZXSHADY_NODISCARD const_iterator         end()     const noexcept { return cend(); }
    ZXSHADY_NODISCARD const_iterator         cbegin()  const noexcept { return const_iterator(&mNumbers[0], 0); }
    ZXSHADY_NODISCARD const_iterator         cend()    const noexcept { return const_iterator(&mNumbers.back(), static_cast<int>(math::digit_count(mNumbers.back()))); }
    ZXSHADY_NODISCARD reverse_iterator       rbegin()        noexcept { return reverse_iterator(end()); }
    ZXSHADY_NODISCARD reverse_iterator       rend()          noexcept { return reverse_iterator(begin()); }
    ZXSHADY_NODISCARD const_reverse_iterator rbegin()  const noexcept { return crbegin(); }
    ZXSHADY_NODISCARD const_reverse_iterator rend()    const noexcept { return crend(); }
    ZXSHADY_NODISCARD const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    ZXSHADY_NODISCARD const_reverse_iterator crend()   const noexcept { return const_reverse_iterator(begin()); }

    bool is_prime() const noexcept;
    bool is_even() const noexcept { return mNumbers[0] % 2 == 0; }
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
    void reverse() &
    {
        auto str = to_string();
        auto begin = str.crbegin();
        auto end = str.crend();

        if (str[0] == '-')
            --end;
        *this = bigint( begin,end );
        this->set_sign(str[0] == '-');
    }

    bigint& half() & noexcept;
    bigint half() && noexcept
    {
        return this->half(); // calls lvalue overload of half();
    }

    bigint& double_() &;
    // calls lvalue overload of double_();
    bigint  double_() && { return this->double_(); } 
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
    friend std::ostream& operator<<(std::ostream& ostream, const bigint& bignum);
    friend std::istream& operator>>(std::istream& istream, bigint& bignum);


    static bigint pow10(unsigned long long exponent);

    static bigint rand(std::size_t num_digits = 1000);

    /// @brief compares 
    /// @return -1 if a  <  b
    /// @return  0 if a  == b
    /// @return +1 if a  >  b
    template<typename T>
    int compare(T that) const noexcept
    {
        auto& a = *this;
        auto& b = that;

        if (a < b)
            return -1;
        if (a == b)
            return 0;
        return 1;
    }

    /// @brief compares 
    /// @return -1 if a  <  b
    /// @return  0 if a  == b
    /// @return +1 if a  >  b
    int compare(const bigint& that) const noexcept
    {
        return compare<const bigint&>(that);
    }

    /// @brief compares using absolute value faster than abs(a).compares(abs(b))
    /// @return -1 if a  <  b
    /// @return  0 if a  == b
    /// @return +1 if a  >  b
    template<typename Integer>
    int signless_compare(Integer that) const noexcept
    {
        return signless_compare(bigint{ that });
        //auto& a = *this;
        //auto b = math::unsigned_abs(that);
        //if (a < b)
        //    return -1;
        //if (bigint::eq(a, b, false, false))
        //    return 0;
        //return 1;
    }

    /// @brief compares using absolute value faster than abs(a).compares(abs(b))
    /// @return -1 if a  <  b
    /// @return  0 if a  == b
    /// @return +1 if a  >  b
    int signless_compare(const bigint& that) const noexcept
    {
        auto& a = *this;
        auto& b = that;
        if (bigint::lt(a, b, false, false))
            return -1;
        if (bigint::eq(a, b, false, false))
            return 0;
        return 1;
    }

    ZXSHADY_NODISCARD bool signless_lt(const bigint& that) const noexcept
    {
        return bigint::lt(*this, that, false, false);
    }
    ZXSHADY_NODISCARD bool signless_gt(const bigint& that) const noexcept
    {
        return bigint::lt(that,*this, false, false);
    }

    ZXSHADY_NODISCARD bool signless_lteq(const bigint& that) const noexcept
    {
        return !this->signless_gt(that);
    }
    ZXSHADY_NODISCARD bool signless_gteq(const bigint& that) const noexcept
    {
        return !this->signless_lt(that);
    }

    template<typename Integer>
    ZXSHADY_NODISCARD bool signless_lteq(Integer that) const noexcept
    {
        return !this->signless_gt(that);
    }
    template<typename Integer>
    ZXSHADY_NODISCARD bool signless_gteq(Integer that) const noexcept
    {
        return !this->signless_lt(that);
    }

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    ZXSHADY_NODISCARD bool signless_lt(Integer that) const noexcept
    {
        auto acount = digit_count();
        auto bcount = zxshady::math::digit_count(that);

        if (acount != bcount)
            return (acount < bcount);

        auto unsigned_that = math::unsigned_abs(that);

        if ZXSHADY_CONSTEXPR17(sizeof(Integer) < sizeof(number_type))
            return mNumbers[0] < unsigned_that;
        else {
            decltype(unsigned_that) x{ 0 };

            const auto end = mNumbers.crend();
            for (auto iter = mNumbers.crbegin(); iter != end; ++iter)
                x = static_cast<decltype(unsigned_that)>(*iter + x * kMaxDigitsInNumber);
            return x < unsigned_that;
        }
    }

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    ZXSHADY_NODISCARD bool signless_gt(Integer that) const noexcept
    {
        auto acount = digit_count();
        auto bcount = zxshady::math::digit_count(that);

        if (acount != bcount)
            return (acount < bcount);

        auto unsigned_that = math::unsigned_abs(that);

        if ZXSHADY_CONSTEXPR17(sizeof(Integer) < sizeof(number_type))
            return mNumbers[0] < unsigned_that;
        else {
            decltype(unsigned_that) x{ 0 };

            const auto end = mNumbers.crend();
            for (auto iter = mNumbers.crbegin(); iter != end; ++iter)
                x = static_cast<decltype(unsigned_that)>(*iter + x * kMaxDigitsInNumber);
            return unsigned_that < x;
        }
    }

    friend bigint fac(bigint x);

private:
    static int parseDigit(char digit,int base = 10)
    {
        constexpr unsigned char table[] = {
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            0,1,2,3,4,5,6,7,
            8,9,255,255,255,255,255,255,
            255,10,11,12,13,14,15,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,10,11,12,13,14,15,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255,255,
            255,255,255,255,255,255,255
        };
        static_assert(sizeof(table) == 255, "table size is not correct");
        int value = table[digit - CHAR_MIN];
        if (value >= base) {
            return 255;
        }
        return value;
    }
    // internal functions for deciding the sign 
    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    static bigint mul(const bigint& a, Integer b, bool a_negative, bool b_negative);
    
    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    static bigint div(const bigint& a, Integer b, bool a_negative, bool b_negative);
    
    //template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    //static bigint sub(Integer a, const bigint& b, bool a_negative, bool b_negative)
    //{
    //    return sub(bigint{ a }, b, b_negative, a_negative);
    //}

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    static bigint mul(Integer a, const bigint& b, bool a_negative, bool b_negative)
    {
        return mul(b, a, b_negative, a_negative);
    }

    static bool eq(const bigint& a, const bigint& b, bool a_negative, bool b_negative) noexcept
    {
        return a_negative == b_negative && a.mNumbers == b.mNumbers;
    }


    static bool lt(const bigint& a, const bigint& b, bool a_negative, bool b_negative) noexcept;
    static void add_compound(bigint& a, const bigint& b, bool a_negative, bool b_negative);
    static void sub_compound(bigint& a, const bigint& b, bool a_negative, bool b_negative);
    static bigint mul(const bigint& a, const bigint& b, bool a_negative, bool b_negative);
    static bigint div(const bigint& a, const bigint& b, bool a_negative, bool b_negative);

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    static void add_compound(bigint& a, Integer b, bool a_negative, bool b_negative);

    template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
    static void sub_compound(bigint& a, Integer b, bool a_negative, bool b_negative);

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
    storage_type mNumbers;
    bool mIsNegative;
};

using signed_bigint = zxshady::bigint;


inline bool is_negative(const bigint& x) noexcept {
    return x.is_negative();
}

inline bool is_positive(const bigint& x) noexcept {
    return x.is_positive();
}


// optimized function overloads

ZXSHADY_NODISCARD inline bool operator==(const bigint& a,const std::string& b) noexcept { return a == bigint(b);}
ZXSHADY_NODISCARD inline bool operator==(const bigint& a, const char* b) noexcept { return a == bigint(b); }
ZXSHADY_NODISCARD inline bool operator==(const std::string& a, const bigint& b) noexcept { return b == a; }
ZXSHADY_NODISCARD inline bool operator==(const char* a, const bigint& b) noexcept { return b == a; }

ZXSHADY_NODISCARD inline bool operator<(const bigint& a,const std::string& b) noexcept { return a < bigint(b);}
ZXSHADY_NODISCARD inline bool operator<(const bigint& a, const char* b) noexcept { return a < bigint(b); }

ZXSHADY_NODISCARD inline bool operator<(const std::string& a,const bigint& b) noexcept { return bigint(a) < b;}
ZXSHADY_NODISCARD inline bool operator<(const char* a,const bigint& b) noexcept { return bigint(a) < b; }


ZXSHADY_DEFINE_COMPARISONS(const bigint&, const bigint&)
ZXSHADY_DEFINE_COMPARISONS(const bigint&, const std::string&)
ZXSHADY_DEFINE_COMPARISONS(const bigint&, const char*)
ZXSHADY_DEFINE_COMPARISONS(const std::string&,const bigint&)
ZXSHADY_DEFINE_COMPARISONS(const char*,const bigint&)

ZXSHADY_DEFINE_ARITHMETIC(bigint,const bigint&)

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator+(bigint a, Integer b)
{
    return a += b;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator+(Integer a,bigint b)
{
    return b += a;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator-(bigint a, Integer b)
{
    return a -= b;
}

// relies on implicit conversion of left hand side...
//template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
//ZXSHADY_NODISCARD bigint operator-(Integer a, const bigint& b)
//{
//    return bigint{ a } - b;
//}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator*(bigint a, Integer b)
{
    return a *= b;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator*(Integer a, bigint b)
{
    return b *= a;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator/(bigint a,Integer b)
{
    return a /= b;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0>
ZXSHADY_NODISCARD bigint operator%(bigint a,Integer b)
{
    return a %= b;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 > ZXSHADY_NODISCARD bool operator!=(Integer a, const bigint& b) noexcept { return !(a == b); }
template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 > ZXSHADY_NODISCARD bool operator!=(const bigint& a, Integer b) noexcept { return !(a == b); }



template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>(const bigint& a,Integer b) noexcept
{
    return b < a;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator<=(const bigint& a,Integer b) noexcept
{
    return !(a > b);
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>=(const bigint& a,Integer b) noexcept
{
    return !(a < b);
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>(Integer a,const bigint& b) noexcept
{
    return b < a;
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator<=(Integer a,const bigint& b) noexcept
{
    return !(a > b);
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type = 0 >
ZXSHADY_NODISCARD bool operator>=(Integer a,const bigint& b) noexcept
{
    return !(a < b);
}

inline void swap(bigint& a,bigint& b) noexcept
{
    a.swap(b);
}

/// @brief makes a copy of *this (NOTE to make it positive use bigint::abs(*this))
/// @return a copy of *this
ZXSHADY_NODISCARD inline bigint operator+(bigint x)
{return x;};

/// @brief negates sign
/// @return negates the sign and returns a copy
ZXSHADY_NODISCARD inline bigint operator-(bigint x)
{x.flip_sign();return x;};


inline bigint abs(bigint x)
{
    x.set_positive();
    return x;
}

inline bigint fac(bigint x)
{
    if (x.is_negative())
        throw std::invalid_argument("zxshady::fac(bigint x) input cannot be negative.");
    // assumes no one is going to try factorial of number bigger than long max
    // if you REALLY really want it then just make
    const auto as_long_long = x.template to<unsigned long long>();
    // 3
    // 3 * (2-1)
    x.mNumbers.reserve(1 + (as_long_long*5) / bigint::kDigitCountOfMax);
    bigint y = x;
    for (std::size_t i = 2; i <= as_long_long; ++i) {
        --y;
        x *= y;
    }
    return x;
}



unsigned long long log2(bigint x);
unsigned long long log10(const bigint& x);
unsigned long long log(const bigint& x, unsigned long long base);
bigint pow(bigint base, unsigned long long exponent);
bigint sqrt(bigint x);
bigint gcd(bigint a, bigint b);
bigint lcm(const bigint& a, const bigint& b);



inline namespace literals {

/* left out... for being utterly useless
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
        */
        inline namespace bigint_literals {

        inline ::zxshady::bigint operator""_big(unsigned long long x)
        {
            return bigint{ x };
        }

        inline ::zxshady::bigint operator""_big(const char* str, std::size_t len)
        {
            return bigint{ str,str + len };
        }

        }
    }











#include "bigint.inl"

}






#endif // !ZXSHADY_BIGINT_HPP



