
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <iostream>
#include <random>
#include <limits>

#include "bigint.hpp"

using namespace zxshady;

bigint::bigint(const char* begin,const char* end)
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

bigint zxshady::sqrt(const bigint& x)
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


bigint bigint::add(const bigint& a, const bigint& b, bool a_negative, bool b_negative) {
    
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

bigint bigint::sub(const bigint& a, const bigint& b, bool a_negative, bool b_negative)
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
    for (; it1 != end; ++it1, void(), ++it2)
        if (*it1 != *it2)
            return (*it1 < *it2) != a_negative;
    
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

bigint bigint::pow10(unsigned long long exponent)
{
    if (exponent == 0)
        return bigint{ 1 };
    bigint ret;
    ret.mNumbers.resize(exponent / kDigitCountOfMax + 1);
    ret.mNumbers[exponent / kDigitCountOfMax] = math::pow10(exponent % kDigitCountOfMax);
    return ret;
}

bool zxshady::bigint::is_prime() const noexcept
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
    return ostream << bigint.to_string();
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

