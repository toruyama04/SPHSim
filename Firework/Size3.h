#pragma once
#include <initializer_list>

class Size3
{
public:
	size_t x;
	size_t y;
	size_t z;

	constexpr Size3() : x(0), y(0), z(0) {}
	constexpr Size3(size_t x_, size_t y_, size_t z_) : x(x_), y(y_), z(z_) {}

    //! Returns reference to the \p i -th element of the size.
    size_t& operator[](size_t i);

    //! Returns const reference to the \p i -th element of the size.
    const size_t& operator[](size_t i) const;

    //! Set x, y, and z components with given initializer list.
    template <typename U>
    Size3& operator=(const std::initializer_list<U>& lst);

    //! Set x, y, and z with other size \p pt.
    Size3& operator=(const Size3& v);

    //! Computes this += (v, v, v)
    Size3& operator+=(size_t v);

    //! Computes this += (v.x, v.y, v.z)
    Size3& operator+=(const Size3& v);

    //! Computes this -= (v, v, v)
    Size3& operator-=(size_t v);

    //! Computes this -= (v.x, v.y, v.z)
    Size3& operator-=(const Size3& v);

    //! Computes this *= (v, v, v)
    Size3& operator*=(size_t v);

    //! Computes this *= (v.x, v.y, v.z)
    Size3& operator*=(const Size3& v);

    //! Computes this /= (v, v, v)
    Size3& operator/=(size_t v);

    //! Computes this /= (v.x, v.y, v.z)
    Size3& operator/=(const Size3& v);

    //! Returns true if \p other is the same as this size.
    bool operator==(const Size3& v) const;

    //! Returns true if \p other is the not same as this size.
    bool operator!=(const Size3& v) const;
};

//! Positive sign operator.
Size3 operator+(const Size3& a);

//! Negative sign operator.
Size3 operator-(const Size3& a);

//! Computes (a, a, a) + (b.x, b.y, b.z).
Size3 operator+(size_t a, const Size3& b);

//! Computes (a.x, a.y, a.z) + (b.x, b.y, b.z).
Size3 operator+(const Size3& a, const Size3& b);

//! Computes (a.x, a.y, a.z) - (b, b, b).
Size3 operator-(const Size3& a, size_t b);

//! Computes (a, a, a) - (b.x, b.y, b.z).
Size3 operator-(size_t a, const Size3& b);

//! Computes (a.x, a.y, a.z) - (b.x, b.y, b.z).
Size3 operator-(const Size3& a, const Size3& b);

//! Computes (a.x, a.y, a.z) * (b, b, b).
Size3 operator*(const Size3& a, size_t b);

//! Computes (a, a, a) * (b.x, b.y, b.z).
Size3 operator*(size_t a, const Size3& b);

//! Computes (a.x, a.y, a.z) * (b.x, b.y, b.z).
Size3 operator*(const Size3& a, const Size3& b);

//! Computes (a.x, a.y, a.z) / (b, b, b).
Size3 operator/(const Size3& a, size_t b);

//! Computes (a, a, a) / (b.x, b.y, b.z).
Size3 operator/(size_t a, const Size3& b);

//! Computes (a.x, a.y, a.z) / (b.x, b.y, b.z).
Size3 operator/(const Size3& a, const Size3& b);

//! Returns element-wise min size.
Size3 min(const Size3& a, const Size3& b);

//! Returns element-wise max size.
Size3 max(const Size3& a, const Size3& b);

//! Returns element-wise clamped size.
Size3 clamp(const Size3& v, const Size3& low, const Size3& high);

//! Returns element-wise ceiled size.
Size3 ceil(const Size3& a);

//! Returns element-wise floored size.
Size3 floor(const Size3& a);

