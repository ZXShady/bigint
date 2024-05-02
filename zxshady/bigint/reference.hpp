#ifndef ZXSHADY_BIGINT_REFERENCE
#define ZXSHADY_BIGINT_REFERENCE

#include "../../zxshady/math.hpp"

#include <type_traits>

namespace zxshady {
namespace details {
namespace bigint {

template<typename T>
class reference {
public:
    constexpr reference(T* ptr, int place) noexcept : m_ptr(ptr), m_place(place) {};
    constexpr reference(const reference&) = default;

    void operator=(const reference&) = delete;

    template<bool CanModify = !std::is_const<T>::value,typename std::enable_if<CanModify,int>::type = 0>
    reference& operator=(int value) noexcept
    {
        math::set_nth_digit(*m_ptr, value, m_place);
        return *this;
    }

    operator T() const noexcept
    {
        return math::get_nth_digit(*m_ptr, m_place);
    }

private:
    T* m_ptr = nullptr;
    int m_place = 0;
};

}
}
}
#endif // !defined(ZXSHADY_BIGINT_REFERENCE)