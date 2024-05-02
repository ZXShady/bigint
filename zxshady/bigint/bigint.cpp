
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <iostream>
#include <random>
#include <limits>
#include <memory>
#include <sstream>

#include "bigint.hpp"

using namespace zxshady;

template<typename T,std::size_t N>
static constexpr std::size_t size(const volatile T(&)[N]) noexcept
{
    return N;
}

template<typename Integer>
static void int_into_stream(Integer x,std::ostream& ostream)
{
    char buffer[1 + zxshady::math::constexpr_log((std::numeric_limits<Integer>::max)())];
    int i = 0;
    do {
        buffer[i] = (x % 10) + '0';
        i++;
        x /= 10;
    } while (x != 0);
    for (; i > 0; --i) {
        ostream.put(buffer[i-1]);
    }
}

template<typename Integer>
static void int_into_stream_hex(Integer x,std::ostream& ostream)
{
    static constexpr const char table[] = {
        '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
    };
    
    static_assert(sizeof(table) == 16, "table size different");
    do {
        ostream.put(table[x % 16]);
        x /= 16;
    } while (x);
}

template<typename Integer>
static void int_into_stream_bin(Integer x,std::ostream& ostream)
{
    do {
        ostream.put((x % 2) + '0');
        x /= 2;
    } while (x);
}
bigint zxshady::sqrt(bigint x)
{
    if (x.is_negative())
        throw std::domain_error("zxshady::sqrt(const bigint& x) x must not be negative");

    // 1 4 9 16 25 36
    if (!x)
        return bigint{ 0 };

    if (x < static_cast<unsigned char>(4))
        return 1;
    if (x < static_cast<unsigned char>(9))
        return 2;
    if (x < static_cast<unsigned char>(16))
        return 3;
    if (x < static_cast<unsigned char>(25))
        return 4;
    if (x < static_cast<unsigned char>(36))
        return 5;
    if (x < static_cast<unsigned char>(49))
        return 6;
    if (x < static_cast<unsigned char>(64))
        return 7;


    bigint x0 = 1;
    bigint x1;
start:
    x1 = ((x / x0) + x0).half();
    if (x0 == x1 || x0 == (x1 - 1)) {
        return x0;
    }
    x0 = std::move(x1);
    goto start;
}

bool bigint::is_pow_of_10() const noexcept
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
bool bigint::is_pow_of_2() const noexcept
{
    if (!*this) // == 0
        return true;

    auto x = *this;

    while (x.is_even())
        x.half();
    return x == (unsigned char)1;
}
std::string bigint::to_string() const
{
    std::stringstream s;
    s << *this;
    return s.str();
}

void bigint::add_compound(bigint& a, const bigint& b, bool a_negative, bool b_negative) {
    
    if (!a_negative && b_negative) {
        // a + -b = a - b 
        sub_compound(a, b, false, false);
        return;
    }
    
    if (a_negative && !b_negative) {
        // -a + b = b - a 
        bigint x = b;
        sub_compound(x, a, false, false);
        a = std::move(x);
        return;
    }
    
    if (a_negative && b_negative) {
        // -a + -b = -(b + a)
        add_compound(a, b, false, false);
        a.set_negative();
        return;
    }

    number_type carry = 0;
    
    const std::size_t asize = a.mNumbers.size();
    const std::size_t bsize = b.mNumbers.size();
    const std::size_t max = (std::max)(asize, bsize);
    const std::size_t min = (std::min)(asize, bsize);
    std::size_t i = 0;
    for (; i < min; ++i) {
        number_type sum = carry;
        sum += a.mNumbers[i];
        sum += b.mNumbers[i];

        a.mNumbers[i] = sum % kMaxDigitsInNumber;
        carry = sum / kMaxDigitsInNumber;
    }

    for (; i < max; ++i) {
        number_type sum = carry;
        if(i < bsize)
            sum += b.mNumbers[i];

        if (i < asize) {
            sum += a.mNumbers[i];
            a.mNumbers[i] = sum % kMaxDigitsInNumber;
        }
        else {
            a.mNumbers.push_back(sum % kMaxDigitsInNumber);
        }
        carry = sum / kMaxDigitsInNumber;
    }

    if (carry != 0) {
        a.mNumbers.push_back(carry);
    }

    a.fix();
}

void bigint::sub_compound(bigint& a, const bigint& b, bool a_negative, bool b_negative)
{
    if (!a_negative && b_negative) {
        // a - -b == a + b
        bigint::add_compound(a, b, false, false);
        return;
    }

    if (a_negative && !b_negative) {
        // -a - b == -(a+b)
        bigint::add_compound(a, b, false, false);
        a.set_negative();
        return;
    }

    if (a_negative && b_negative) {
        // -a - -b == (b - a)
        // -2 - -3 == -2 + 3 = 3 - 2;
        bigint x = b;
        bigint::sub_compound(x, a, false, false);
        a = std::move(x);
        return;
    }

    if (a.signless_lt(b)) {
        bigint x = b;
        bigint::sub_compound(x,a,false,false);
        a = std::move(x);
        a.set_sign(!a_negative);
        return;
    }

    //   2132
    // -  343
    // 
    //  132
    //  343
    // -211 (+1000)
    // +899
    //  1 - 0

    std::int64_t carry = 0;
    const std::size_t asize = a.mNumbers.size();
    const std::size_t bsize = b.mNumbers.size();
    const std::size_t min = (std::min)(asize, bsize);
    const std::size_t max = (std::max)(asize, bsize);

    std::size_t i = 0;
    for (; i < min; i++) {
        std::int64_t diff = carry;

        diff += static_cast<std::int64_t>(a.mNumbers[i]);
        diff -= static_cast<std::int64_t>(b.mNumbers[i]);

        if (diff < 0) {
            diff += kMaxDigitsInNumber;
            carry = -1;
        }
        else {
            carry = 0;
        }
        a.mNumbers[i] = static_cast<number_type>(diff % kMaxDigitsInNumber);
    }

    for (; i < max; i++) {
        std::int64_t diff = carry;

        if(i < asize)
           diff += static_cast<std::int64_t>(a.mNumbers[i]);
        if(i < bsize)
            diff -= static_cast<std::int64_t>(b.mNumbers[i]);

        if (diff < 0) {
            diff += kMaxDigitsInNumber;
            carry = -1;
        }
        else {
            carry = 0;
        }

        a.mNumbers[i] = static_cast<number_type>(diff % kMaxDigitsInNumber);
    }

    a.fix();
}

bigint bigint::mul(const bigint& a, const bigint& b, bool a_negative, bool b_negative) 
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

bigint bigint::div(const bigint& a, const bigint& b, bool a_negative, bool b_negative)
{
    if (!b)
        throw std::invalid_argument("zxshady::bigint::operator/ Division by zero.");

    // if the abs(a) is less than abs(b), the quotient is 0
    if (bigint::lt(a, b, false, false)) {
        return bigint{};
    }


    bigint current{};  // current number in process of division
    bigint ret{ noinit_t{} };
    ret.mNumbers.resize(a.mNumbers.size());
    auto index = ret.mNumbers.size() - 1;
    const auto it_end = a.mNumbers.crend();
    for (auto iter = a.mNumbers.crbegin(); iter != it_end; ++iter) {
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
        ret.mNumbers[index] = end;
        --index;
        current -= end * b;
    }

    ret.mIsNegative = a_negative != b_negative;
    ret.fix();
    return ret;
}
bool bigint::lt(const bigint& a, const bigint& b,bool a_negative,bool b_negative) noexcept
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
    for (; it1 != end; ++it1, void(), ++it2) {
        const auto x = *it1;
        const auto y = *it2;
        if (x != y)
            return (x < y) != a_negative;
    }
    return false;
}
bigint& bigint::half() & noexcept
{
    if (this->is_positive() && *this <= 2) {
        auto& first = mNumbers[0];
        first = first == 2;
        return *this;
    }

    bigint current{};  // current number in process of division
    storage_type res;
    res.reserve(mNumbers.size());

    const auto it_end = mNumbers.crend();
    // 513 
    //   2
    // 256

    for (auto iter = mNumbers.crbegin(); iter != it_end; ++iter) {
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

bigint& bigint::double_() & 
{
    number_type carry = 0;

    const std::size_t size = mNumbers.size();
    for (std::size_t i = 0; i < size; ++i) {
        number_type sum = carry;
        sum += 2 * mNumbers[i];
        mNumbers[i] = sum % kMaxDigitsInNumber;
        carry = sum / kMaxDigitsInNumber;
    }
    if (carry > 0)
        mNumbers.push_back(carry);
    return *this;
}



bigint bigint::rand(std::size_t num_digits /* = 1000 */)
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
unsigned long long zxshady::log2(bigint x)
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
unsigned long long zxshady::log10(const bigint& x)
{
    if(!x)
        throw std::invalid_argument("zxshady::bigint::log10(bigint x) input cannot be 0");
    if(x.is_negative())
        throw std::invalid_argument("zxshady::bigint::log10(bigint x) input cannot be negative");

    return x.digit_count() - 1;
}
unsigned long long zxshady::log(const bigint& x,unsigned long long base )
{
    if(!x)
        throw std::invalid_argument("zxshady::bigint::log(bigint x,unsigned long long base) x cannot be 0");

    if(x.is_negative())
        throw std::invalid_argument("zxshady::bigint::log(bigint x,unsigned long long base) x cannot be negative");

    if(base <= 1)
        throw std::invalid_argument("zxshady::bigint::log(bigint x,unsigned long long base) base cannot be less than or equal to 1");


    return static_cast<unsigned long long>(static_cast<double>(log10(x)) / std::log10(base));
}
bigint zxshady::pow(bigint base, unsigned long long exponent)
{
    if (!base)
        return bigint{};

    if (base == static_cast<unsigned char>(10)) // short cut for faster execution
        return bigint::pow10(exponent);

    if (base == static_cast<unsigned char>(1))
        return bigint{ 1 };

    if (exponent == 0)
        return bigint{ 1 };

    if (exponent == 1)
        return base;

    const bool was_negative = exponent % 2 == 1 && base.is_negative();
    base.set_positive();
    const bigint mult = base;
    while (exponent > 1) {
        base *= mult;
        --exponent;
    }
 
    base.set_sign(was_negative);
    return base;
}
bigint bigint::pow10(unsigned long long exponent)
{
    if (exponent == 0)
        return bigint{ 1 };
    bigint ret;
    ret.mNumbers.resize(exponent / kDigitCountOfMax + 1);
    ret.mNumbers[exponent / kDigitCountOfMax] = static_cast<number_type>(math::pow10(exponent % kDigitCountOfMax));
    return ret;
}
bool zxshady::bigint::is_prime() const noexcept
{
    if (*this == static_cast<unsigned char>(2))
        return true;

    if (is_negative() || is_even() || !*this || !(*this % static_cast<unsigned char>(3)))
        return false;


    bigint range = sqrt(*this);
    for (bigint i = 5; i <= range; i += static_cast<unsigned char>(6))
        if (!(*this % i) || !(*this % (i + static_cast<unsigned char>(2))))
            return false;
    return true;
}
bigint zxshady::gcd(bigint a, bigint b)
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
bigint zxshady::lcm(const bigint& a, const bigint& b)
{
    if (!a || !b ) // using negation operator! to check for == 0 since it is hardcoded it is faster!
        return bigint{};
    return abs(a * b) / gcd(a, b);

}
std::ostream& zxshady::operator<<(std::ostream& ostream, const zxshady::bigint& bigint)
{

    if (bigint.is_negative()) {
        ostream << '-';
    }
    else if ((ostream.flags() & std::ios_base::showpos) != 0) {
        if (bigint /* != 0 */)
            ostream << '+';
    }
    const bool showbase = (ostream.flags() & std::ios_base::showbase) != 0;
    const bool uppercase = (ostream.flags() & std::ios_base::uppercase) != 0;

    std::ios_base::fmtflags base = ostream.flags() & ostream.basefield;
    if ((base & std::ios_base::dec) != 0) {
        auto iter = bigint.mNumbers.crbegin();
        const auto end = bigint.mNumbers.crend();

        int_into_stream(*iter, ostream);
        ++iter;
        for (; iter != end; ++iter) {
            auto val = *iter;
            auto cnt = zxshady::math::digit_count(val);
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
            ostream << zeroes[difference];
            int_into_stream(*iter, ostream);

        }
    }
    else if ((base & std::ios_base::hex) != 0) {
        
        auto x = bigint;
        std::size_t size = sizeof(bigint::number_type) * CHAR_BIT * bigint.mNumbers.size();
        std::unique_ptr<char[]> buffer(new char[size]);
        std::ptrdiff_t i = 0;
        
        if (showbase) {
            ostream.put('0');
            ostream.put(uppercase ? 'X' : 'x');
        }

        constexpr static char table_lower[] = {
            '0','1','2','3','4','5','6','7',
            '8','9','a','b','c','d','e','f'
        };

        constexpr static char table_upper[] = {
            '0','1','2','3','4','5','6','7',
            '8','9','A','B','C','D','E','F'
        };
        static_assert(sizeof(table_lower) == 16, "wrong size");
        static_assert(sizeof(table_upper) == 16, "wrong size");
        // 17
        const auto& table = uppercase ? table_upper : table_lower;
        do {

            buffer[static_cast<std::size_t>(i)] = table[x.mNumbers[0] % 16];
            i++;
            x /= static_cast<unsigned char>(16);
        } while (x);

        for (; i > 0; --i) {
            ostream.put(buffer[i-1]);
        }
    }
    else if((base & std::ios_base::binary) != 0){
        auto x = bigint;
        std::unique_ptr<char[]> buffer(
            new char[sizeof(bigint::number_type) * CHAR_BIT * bigint.mNumbers.size()]);
        std::ptrdiff_t i = 0;

        if (showbase) {
            ostream.put('0');
            ostream.put(uppercase ? 'B' : 'b');
        }

        do {
            buffer[static_cast<std::size_t>(i)] = x.is_odd() + '0';
            i++;
            x.half();
        } while (x);

        for (; i > 0; --i) {
            ostream.put(buffer[i-1]);
        }
    }
    return ostream;
}
std::istream& zxshady::operator>>(std::istream& istream, zxshady::bigint& bigint)
{
    std::string s;
    istream >> s;
    if (istream.good())
        bigint = s;
    else
        bigint.zero();
    return istream;
}

