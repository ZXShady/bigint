# BigInt
## Big Integer Library for C++
### bigint is a C++ library which can handle Very very well very __Big Integers__. It can do the *factorial* of __1000000...__ (given enough time) it can go very big it can do pow(15,1351)...

---

# How to use it?
```c++
#include "bigint.hpp"     // with proper file path and link bigint.cpp
// or just
#include "single_header_bigint.hpp" // no linking needed
```
---

# Declaring and Intializing Variables.

Declartion is done like making object of bigint class.
Intialization can be done by passing *String* or *Integer* type at Object creation.

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
bigint l{0} // is slower than two above
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
c -= "0000983109813409810948109384"; // will ignore leading zeroes
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
11. bool is_pow_of_10() const noexcept;
12. bool is_pow_of_2() const noexcept;
13. std::string to_string() const;
14. std::size_t digit_count() const noexcept;
15. std::size_t bit_count();
16. int compare(const bigint& that) const noexcept // a memcpy like function
// returns
// a number *< 0* if *this < 0
// a *0* if *this == that    
// a number *> 0* if *this > that

17. int signless_compare(const bigint& that) // same as above but compares as if they were abs(*this).compare(abs(that));
// faster than using abs method
18. void swap(bigint& that) noexcept; 
```

# Static Functions
```cpp
    static bigint pow10(unsigned long long exponent); // gives bigint with [exponent] trailing zeroes
    static bigint rand(std::size_t digits = 1000); // random bigint [with default = 1000] 
    static bigint add(const bigint& a, const bigint& b); // same as operator+ but named for java programmers :P
    static bigint sub(const bigint& a, const bigint& b); // same as operator- but named for java programmers :P
    static bigint mul(const bigint& a, const bigint& b); // same as operator* but named for java programmers :P
    static bigint div(const bigint& a, const bigint& b); // same as operator/ but named for java programmers :P
    static bigint mod(const bigint& a, const bigint& b); // same as operator% but named for java programmers :P

```
---

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
#User Defined Literals
```cpp
// literal constant functions useful if you want clear syntax while comparing against zero and some other functions
// bigint a{2};
// if(a == 0); // slow
// if(!a) // fast
// if(a == 0_c) // fast
std::integral_constant<int,(value)> operator""_c();
std::integral_constant<unsigned int,(value)> operator""_uc();
std::integral_constant<long,(value)> operator""_lc();
std::integral_constant<long long,(value)> operator""_llc();
std::integral_constant<unsigned long,(value)> operator""_ulc();
std::integral_constant<unsigned long long,(value)> operator""_ullc();
// they are found in the inlined namespace const_literals
// you can bring them with a using declartion
void func() {
  using zxshady::literals::const_literals;
  bigint x{0};
  if(x == 0_c)                             return;
    harddrive.format();
}


bigint operator""_big(unsigned long long x); // equalivent to bigint(x);
bigint operator""_big(const char* str,std::size_t len); // equalivent to bigint(str,str+len);
// these operators live in bigint_literals inline namespace

usage
using namespace zxshady::literals::bigint_literals; // or zxshady::bigint_literals;
bigint val = "120948190380984093810948190348813409813571980709795748758798"_big;
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
const bigint x = 21982014;
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




