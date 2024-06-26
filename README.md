# BigInt
## Big Integer Library for C++ 11 and above
### bigint is a C++ library which can handle *very very very* __Big Integers__. It can do the *factorial* of __1000000...__ (given enough time) it can go very big it can do pow(15,1351)...

---

# How to use it?
```c++
#include "zxshady/bigint/bigint.hpp"     // with proper file path and link bigint.cpp
```
---

# Declaring and Intializing Variables.

Declartion is done like making object of bigint class.
Intialization can be done by passing *String* or *Integer* (including extended) types at Object creation.

```c++
bigint a("1538901385913857893173895103987501387");     // big integer initialization with String
bigint b(std::string("-351901380593"));        // big integer initialization with String
bigint c(956486133);                                    // big integer intialization with Integer
bigint d = 129084091248;

// bigint e = "1209841092" // error the string constructor is explicit to prevent unnecesary conversions
// bigint f = std::string("12091204"); // same as above
std::string str = "hi 123";
bigint g(str.c_str() + 3,str.c_str() + str.size()); //range constructor constructs begining at "123"
bigint h(str.c_str()+3,3) // starts at str.c_str()+3 then reads next 3 characters

// bigint i = nullptr; // error usage of deleted constructor (bigint::bigint(nullptr)= delete)
bigint j{}; // fastest way to initialize to zero
bigint k; // fastest way to initialize to zero
//bigint l(); // silent error it is a function declaration
bigint m{0} // is slower than two above
bigint o{"0xDeAdBeEf"}; // 3735928559
bigint p("0B1101"); // 13
bigint q(static_cast<__int128>(1e30)); // extended integers

str = "ff";
bigint r(str.begin(),str.end(),bigint::base::hex)
str = "1111'1110'0001";
bigint s(str.begin(),str.end(),bigint::base::bin,'\'') // seperator
str = "0xff'ff'ff'ff";
bigint t(str.begin(),str.end(),'\'') // seperator
bigint u("0o70"); // octal
```
---
# avaible operators

1. Addition
2. Subtraction
3. Multiplication
4. Modulo
5. Division
6. Increment
7. Decrement
8. !
9. bool 
10. Unary Plus 
11. Unary Minus 
```c++
zxshady::bigint a= 12,b = 941830318,c{"10984019384143130598013958"};

if(0){
hell:
  *(int*)0; // suicide
}
c += 12; // same as c = c + 12; but it might be faster to use +=
c /= 5;
c -= bigint("0000983109813409810948109384"); // will ignore leading zeroes
c %= 2;
--c; // way faster than c -= 1
c--; // post-fix version is [[nodiscard]] might get warning or error
++c; // way faster than c += 1
c++;  // post-fix version is [[nodiscard]] might get warning or error
c/= 0; // throws std::invalid_arguement
c%= 0;// same as above;
if(!c && !a && b) goto hell;// operator bool to check for non-zero and operator! to check for zero
// it is faster than using c == 0 and b != 0
a = -b; // unary minus returns a copy and flips its sign;
a = +b; // DOES NOT DO abs(b) it just returns a copy...

```
---
# Member Functions

## note if a member function for a specific algrothim or function you want is provided then note that it is faster to use than manual implementation
```cpp
1. bool is_even() const noexcept; // faster than a % 2 == 0
2. bool is_odd() const noexcept; // faster than a % 2 == 1
3. bool is_prime() const noexcept; // slow basic implementation :(
4. void flip_sign() noexcept; // flips the sign faster than a = -a
5. void set_positive() noexcept; // makes the number positive faster than a = abs(a)
6. void set_negative() noexcept; // makes the number negative faster than a = a >= 0 ? -a : a;
7. void set_sign(bool negative); noexcept // sets the sign
8. void reverse(); // reverses the number NOTE: slow function
9. void zero(); // sets the number to 0 faster than a = 0
10. bigint& half(); // halves the number way faster than a /= 2 returns *this for chaining effect
10. bigint& double_(); doubles the number way faster than a *= 2 returns *this for chaning
11. bool is_pow_of_10() const noexcept;
12. bool is_pow_of_2() const noexcept;
13. std::string to_string() const;
14. std::size_t digit_count() const noexcept;
15. std::size_t bit_count() const noexcept;
16. int compare(const bigint& that) const noexcept // a memcpy like function
// returns
// a number *< 0* if *this < that
// a *0* if *this == that    
// a number *> 0* if *this > that

17. int signless_compare(const bigint& that) // same as above but compares as if they were abs(*this).compare(abs(that));
// faster than using abs method
18. void swap(bigint& that) noexcept; 
19. bool signless_NAME(const bigint& that) // NAME could be lt,gt,lteq,gteq
// compares as abs values
```

# Static Functions
```cpp
    static bigint pow10(unsigned long long exponent); // gives bigint with [exponent] trailing zeroes
    static bigint rand(std::size_t digits = 1000); // random bigint [with default = 1000]
```

# IO Functions
It can be formatted like any other integer e.g (print in hex ,show base ,show pos) etc.
```cpp

bigint value = 0xff;
std::cout << std::oct << value; // 377
std::cout << std::dec << std::showpos << value; // +255
std::cout << std::hex << std::showbase << std::showpos << value; // +0xff
std::cout << std::uppercase << value; // 0XFF

bigint x;
std::cin >> x; // enter a number
std::cout << x; // output it!
```

---
# Accessor Functions
```
bigint x = 123;
// 3 is the first digit
// 1 is the last digit
std::cout << x.front(); // 3
std::cout << x.back(); // 1

// range-based for
for(int digit : x) {
    std::cout << digit << ' '; // prints 3 2 1
}

// iterators
const auto end = x.end();
for(auto iter = x.begin();iter != end;++iter) {
    std::cout << *iter << ' '; // prints 3 2 1
}
// reverse iterators
const auto rend = x.rend();
for(auto iter = x.rbegin();iter != rend;++iter) {
    *iter = 4;
    std::cout << *iter << ' '; // prints 4 4 4
}
x = 123;
// const-iterators
const auto cend = x.cend();
for(auto iter = x.cbegin();iter != cend;++iter) {
    std::cout << *iter << ' '; // prints 3 2 1
}
// const-reverse iterators
const auto crend = x.crend();
for(auto iter = x.crbegin();iter != crend;++iter) {
    std::cout << *iter << ' '; // prints 1 2 3
}

x = 100000;
// note auto&& since this container act's like vector<bool>
for(auto&& Value : x) {
   Value = 5;
}
std::cout << x; // x = 55555

x = 123;

bigint::reference first = x[0]; // returns bigint::reference;
// auto first = x[0]; // also works but i wouldnt recommend this since people might think it is a copy...
// auto&& first = x[0]; // recommended or first version (bigint::reference); 
first = 1;
// x = 121



```

# Overloaded Functions

Use following functions with no namespace and ADL will find them automaticly

```cpp

bigint abs(bigint x); // abs function
bigint fac(bigint x);// factorial function
bigint pow(bigint base,unsigned long long exponent);
bigint sqrt(const bigint& x);
bigint gcd(bigint a, bigint b); // greatest commmon denomator
bigint lcm(const bigint& a, const bigint& b);

unsigned long long log2(bigint x);
unsigned long long log10(const bigint& x);
unsigned long long log(const bigint& x, unsigned long long base);

std::ostream& operator<<(std::ostream& ostream, const bigint& bignum); // output operator
std::istream& operator>>(std::istream& istream, bigint& bignum); // input operator

```
# User Defined Literals
```cpp
using namespace zxshady::literals::bigint_literals; // or zxshady::bigint_literals;
auto val = "120948190380984093810948190348813409813571980709795748758798"_big;
pow(100_big,10000);

// to bring all of them at once use
using namespace zxshady::literals;
```
---

# Relational Operators

Conditionals can be used as it is done with general integer types variables.
Supports :  *>* , *<* ,  *>=* , *<=* , *==* , *!=*

```c++
bigint a = 12;
if(a > 12) {
    cout << "a is greater than 12" << std::endl;
} else {
    cout << "a is smaller than 12" << std::endl;
}
// using yoda notation (yikes)
if(12 != a) {
    cout << "a is not equal to 12" << std::endl;
} else {
    cout << "a is equal to 12" << std::endl;        
}
```

---

# converting to integral types using to method
```c++
bigint x = 21982014;
auto val = x.to<int>();// will throw std::range_error if cannot fit inside int;
x.set_negative();
// using unsigned types will just ignore the sign...
auto val = x.to<unsigned int>();// will throw std::range_error if cannot fit inside unsigned int;

// there is also bigint::non_throwing_to<Integral>() which returns a type of zxshady::result<Integeral,std::errc> and is noexcept
zxshady::result<int> val = x.non_throwing_to<int>();
if(val.ec == std::errc()){ // or use if(val) it has convversion to bool
  // can use val.res;
} else {
    if(val.ec == std::errc::result_out_of_range){
    // format harddrive
  }
}
```




