template<typename Integer,
    typename std::enable_if<std::is_integral<Integer>::value,int>::type>
bigint::bigint(Integer num) : mIsNegative(::zxshady::is_negative(num))
{
    auto unsigned_num = math::unsigned_abs<number_type>(num);

    if ZXSHADY_CONSTEXPR17(sizeof(num) < sizeof(number_type))
    {
        mNumbers.push_back(unsigned_num);
    }
    else {
        do {
            mNumbers.push_back(unsigned_num % kMaxDigitsInNumber);
            unsigned_num /= kMaxDigitsInNumber;
        } while (unsigned_num != 0);
    }
}

template<typename InputIter, typename std::enable_if<
    !std::is_integral<InputIter>::value, int>::type>
bigint::bigint(InputIter begin, InputIter end, char seperator)
{
    static_assert(std::is_same<char, typename remove_cvref<decltype(*begin)>::type>::value, "InputIter derefenced must return a char");
    int base = 10;
    if (*begin == '-') {
        mIsNegative = true;
        ++begin;
    }
    else {
        if (*begin == '+')
            ++begin;
        mIsNegative = false;
    }
    if (begin + 1 != end) {
        if (*begin == '0') {
            ++begin;
            switch (*begin) {
                case 'X':
                case 'x':
                    base = bigint::base::hex;
                    break;
                case 'B':
                case 'b':
                    base = bigint::base::bin;
                    break;
                case 'O':
                case 'o':
                    base = bigint::base::oct;
                    break;

            }
            ++begin;
        }
    }
    // "01208021380"
    while (begin != end && *begin == '0')
        ++begin;
    std::reverse_iterator<InputIter> rbegin{ end };
    std::reverse_iterator<InputIter> rend{ begin };
    auto distance = static_cast<std::size_t>(std::distance(begin, end));
    mNumbers.reserve(1 + distance);
    auto it = rbegin;
    switch (base) {
        case bigint::base::dec:
        {
            while (it != rend) {
                number_type num = 0;
                for (std::size_t i = 0; i < kDigitCountOfMax && it != rend; ++i, ++it) {
                    const char Char = *it;
                    const int parsed = parseDigit(Char);
                    if (seperator != bigint::no_seperator && Char == seperator)
                        continue;
                    if (parsed == UCHAR_MAX)
                        throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(std::distance(it, rend)) + " character was " + std::string(1, *it));

                    const auto pow = math::pow(base, i);
                    num += static_cast<number_type>(parsed * pow);
                }
                mNumbers.push_back(num);
            }
            break;
        }
        case bigint::base::bin:
        {
            mNumbers.push_back(0);
            // more efficient if casted to smallest datatype and 
            // using unsigned makes sure to not check for negativness;
            bigint num = static_cast<unsigned char>(1);
            std::size_t index = 0;
            for (; it != rend; ++it) {
                const char Char = *it;
                if (seperator != bigint::no_seperator && Char == seperator)
                    continue;
                int parsed = parseDigit(Char, base);
                if (parsed == UCHAR_MAX)
                    throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(index) + " character was " + std::string(1, Char));

                *this += num * parsed;
                index++;
                num.double_();
            }
            break;
        }
        case bigint::base::hex:
        {
            mNumbers.push_back(0);
            // more efficient if casted to smallest datatype and 
            // using unsigned makes sure to not check for negativness;
            bigint num = static_cast<unsigned char>(1);
            std::size_t index = 0;
            for (; it != rend; ++it) {
                const char Char = *it;
                if (Char == seperator)
                    continue;
                int parsed = parseDigit(Char, base);
                if (parsed == UCHAR_MAX)
                    throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(index) + " character was " + std::string(1, Char));
                *this += num * parsed;
                index++;
                num *= static_cast<unsigned char>(16);
            }
            break;
        }
        case bigint::base::oct:
        {
            mNumbers.push_back(0);
            // more efficient if casted to smallest datatype and 
            // using unsigned makes sure to not check for negativness;
            bigint num = static_cast<unsigned char>(1);
            std::size_t index = 0;
            for (; it != rend; ++it) {
                const char Char = *it;
                if (Char == seperator)
                    continue;
                int parsed = parseDigit(Char, base);
                if (parsed == UCHAR_MAX)
                    throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(index) + " character was " + std::string(1, Char));
                *this += num * parsed;
                index++;
                num *= static_cast<unsigned char>(8);
            }
            break;
        }
    }
    fix();
    if (mNumbers.empty())
        zero();
}


template<typename InputIter, typename std::enable_if<
    !std::is_integral<InputIter>::value, int>::type>
bigint::bigint(InputIter begin, InputIter end, bigint::base base, char seperator)
{
    static_assert(std::is_same<char, typename remove_cvref<decltype(*begin)>::type>::value, "InputIter derefenced must return a char");
    if (*begin == '-') {
        mIsNegative = true;
        ++begin;
    }
    else {
        if (*begin == '+')
            ++begin;
        mIsNegative = false;
    }
    // "01208021380"
    while (begin != end && *begin == '0')
        ++begin;
    std::reverse_iterator<InputIter> rbegin{ end };
    std::reverse_iterator<InputIter> rend{ begin };
    auto distance = static_cast<std::size_t>(std::distance(begin, end));
    mNumbers.reserve(1 + distance / kMaxDigitsInNumber);
    auto it = rbegin;

    switch (base) {
        case bigint::base::dec:
        {
            while (it != rend) {
                number_type num = 0;
                for (std::size_t i = 0; i < kDigitCountOfMax && it != rend; ++i, ++it) {
                    const char Char = *it;
                    const int parsed = parseDigit(Char);
                        if (seperator != bigint::no_seperator && Char == seperator)
                            continue;

                    if (parsed == UCHAR_MAX)
                        throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(std::distance(it, rend)) + " character was " + std::string(1, *it));

                    const auto pow = math::pow(static_cast<int>(base), i);
                    num += static_cast<number_type>(parsed * pow);
                }
                mNumbers.push_back(num);
            }
            break;
        }
        case bigint::base::bin:
        {
            mNumbers.push_back(0);
            // more efficient if casted to smallest datatype and 
            // using unsigned makes sure to not check for negativness;
            bigint num = static_cast<unsigned char>(1);
            std::size_t index = 0;
            for (; it != rend; ++it) {
                const char Char = *it;
                if (seperator != bigint::no_seperator && Char == seperator)
                    continue;
                int parsed = parseDigit(Char);
                if (parsed == UCHAR_MAX)
                    throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(index) + " character was " + std::string(1, Char));
                *this = add(*this, mul(num, parsed));
                index++;
                num.double_();
            }
            break;
        }
        default:
        {
            mNumbers.push_back(0);
            // more efficient if casted to smallest datatype and 
            // using unsigned makes sure to not check for negativness;
            bigint num = static_cast<unsigned char>(1);
            std::size_t index = 0;
            for (; it != rend; ++it) {
                const char Char = *it;
                if (seperator != bigint::no_seperator && Char == seperator)
                    continue;
                int parsed = parseDigit(Char);
                if (parsed == UCHAR_MAX)
                    throw zxshady::bigint_format_error("zxshady::bigint_format_error: invalid character at " + std::to_string(index) + " character was " + std::string(1, Char));
                *this = add(*this, mul(num, parsed));
                index++;
                num = mul(num, static_cast<int>(base));
            }
            break;
        }
    }

    fix();
    if (mNumbers.empty())
        zero();
}

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type>
void bigint::add_compound(bigint& a, Integer b_, bool a_negative, bool b_negative)
{
    // auto required here.
    auto b = math::unsigned_abs<number_type>(b_);
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
        // -a + -b = -(a + b)
        add_compound(a, b, false, false);
        a.set_negative();
        return;
    }

    // 123
    //   9
    // 123
    // 132
    number_type carry = 0;
    const std::size_t size = a.mNumbers.size();
    std::size_t i = 0;
    for (; i < size && b != 0; ++i) {
        number_type sum = carry;
        sum += a.mNumbers[i];
        sum += b % kMaxDigitsInNumber;
        a.mNumbers[i] = sum % kMaxDigitsInNumber;

        b /= kMaxDigitsInNumber;
        carry = sum / kMaxDigitsInNumber;
    }

    for (; i < size && carry; ++i) {
        number_type sum = carry + a.mNumbers[i];
        a.mNumbers[i] = sum % kMaxDigitsInNumber;
        carry = sum / kMaxDigitsInNumber;
    }

    if (carry > 0) {
        a.mNumbers.push_back(carry);
    }
    
    while (b != 0) {
        a.mNumbers.push_back(b % kMaxDigitsInNumber);
        b /= kMaxDigitsInNumber;
    }

    a.fix();
}


template<typename Integer, typename std::enable_if<std::is_integral<Integer>::value,int>::type >
void bigint::sub_compound(bigint& a, Integer b_, bool a_negative, bool b_negative)
{
    auto b = math::unsigned_abs(b_);
    if (!a_negative && b_negative) {
        // a - -b == a + b
        add_compound(a, b, false, false);
        return;
    } 
    if (a_negative && !b_negative) {
        // -a - b == -(a+b)
        add_compound(a, b, false, false);
        a.set_negative();
        return;
    }

    if (a_negative && b_negative) {
        // -a - -b == (b - a)
        bigint x = b;
        sub_compound(x, a,false,false);
        a = std::move(x);
        return;
    }



    // a < b
    // 2 - 3 == -(3 -2)
    if (a.signless_lt(b)) {
        bigint x = b;
        sub_compound(x,a,false,false);
        a = std::move(x);
        a.flip_sign();
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

    const auto size = a.mNumbers.size();
    std::int64_t carry = 0;
    // 123
    //   9
    // 114
    for (std::size_t i = 0; i < size && b != 0; i++) {
        std::int64_t diff = carry;

        diff += static_cast<std::int64_t>(a.mNumbers[i]);
        diff -= static_cast<std::int64_t>(b % kMaxDigitsInNumber);
        b /= kMaxDigitsInNumber;

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

template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type>
bigint bigint::mul(const bigint& a, Integer b_, bool a_negative, bool b_negative)
{
    auto b = math::unsigned_abs<number_type>(b_);
    if (!a || !b) // using operator! to test for == 0 since it is hardcoded it will be faster!
        return bigint{};
    bigint ret{ noinit_t{} };
    const std::size_t asize = a.mNumbers.size();
    const std::size_t bsize = b != 0 ? math::log(b,kMaxDigitsInNumber) + 1
                                     : 1;

    ret.mNumbers.resize(asize + bsize + 1);

    for (std::size_t i = 0; i < asize; i++) {
        std::uint64_t carry = 0;
        b = math::unsigned_abs<number_type>(b_);
        for (std::size_t j = 0; b != 0; j++,b/=kMaxDigitsInNumber) {
            const std::uint64_t product = static_cast<std::uint64_t>(a.mNumbers[i]) * static_cast<std::uint64_t>(b % kMaxDigitsInNumber) +
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


template<typename Integer,typename std::enable_if<std::is_integral<Integer>::value,int>::type>
bigint bigint::div(const bigint& a, Integer b_, bool a_negative, bool b_negative)
{

    if (b_ == 0)
        throw std::invalid_argument("zxshady::bigint::operator/ Division by zero.");

    using DivisorType = typename std::conditional<
        (sizeof(unsigned long long) >= sizeof(number_type) + sizeof(Integer)),
        unsigned long long,
        bigint
        >::type;

    if (a.signless_lt(b_))
        return bigint{};

    DivisorType b = math::unsigned_abs(b_);
    // if the abs(a) is less than abs(b), the quotient is 0


    bigint current{};  // current number in process of division
    bigint ret{ noinit_t{} };
    ret.mNumbers.resize(a.mNumbers.size());
    auto index = ret.mNumbers.size() - 1;
    const auto it_end = a.mNumbers.crend();
    for (auto iter = a.mNumbers.crbegin(); iter != it_end; ++iter) {
        current *= kMaxDigitsInNumber;
        current += *iter;
        number_type start = 0;
        number_type end = kMaxDigitsInNumber;
        while (start <= end) {
            number_type mid = (start + end) / 2;
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
