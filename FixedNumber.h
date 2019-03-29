//-----------------------------------------------------------------------------
//
//  The typical 'half' float16 data type (IEEE 754-2008) uses the following bit
//  allocation: mantissa:10 exponent:5 sign:1.
//  https://en.wikipedia.org/wiki/Half-precision_floating-point_format
//
//  An alternate float16 is essentially float32 (IEEE 754-2008) with the lowest
//  16 of 23 mantissa bits chopped off: mantissa:7 exponent:8 sign:1
//  https://en.wikipedia.org/wiki/Bfloat16_floating-point_format
//
//-----------------------------------------------------------------------------

#pragma once


template <typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
struct FixedNumber
{
    using Self = FixedNumber<typename BaseType, IntegerBitCount, FractionBitCount>;

    constexpr static int32_t TotalBitCount = IntegerBitCount + FractionBitCount;
    constexpr static float FractionMultiple = 1 << FractionBitCount;
    constexpr static float FractionInverseMultiple = 1.0f / FractionMultiple;

    // Check that the base data type uses either 32-bit or 64-bit values.
    // It's fine for them to be signed or unsigned.
    using IntegerType = decltype(BaseType{} + 1);

    FixedNumber() = default;
    FixedNumber(const Self&) = default;
    FixedNumber(Self&&) = default;

    FixedNumber(float newValue) noexcept
    {
        SetFloat(newValue);
    }

    static Self MakeFromRawBits(uint32_t newValue)
    {
        Self self;
        self.SetRawBits(newValue);
        return self;
    }

    Self& operator =(const Self&) = default;

    Self& operator =(float newValue) noexcept
    {
        SetFloat(newValue);
        return *this;
    }

    inline IntegerType GetRawBits()
    {
        return value;
    }

    void SetRawBits(int32_t newValue)
    {
        value = newValue;
    }

    void SetFloat(float newValue)
    {
        value = static_cast<IntegerType>(newValue * FractionMultiple);
    }

    float GetFloat() const noexcept
    {
        return static_cast<float>(IntegerType(value) * FractionInverseMultiple);
    }

    operator float() const noexcept
    {
        return GetFloat();
    }

    BaseType value;
};

//////////////////////////////

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator +(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    a.SetRawBits(a.GetRawBits() + b.GetRawBits());
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator -(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    a.SetRawBits(a.GetRawBits() - b.GetRawBits());
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator *(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    // The product result is bigger than the base type.
    // So we need to keep full precision and then shift and truncate back down.
    int64_t result = int64_t(a.GetRawBits()) * b.GetRawBits();

    using IntegerType = typename FixedNumber<BaseType, IntegerBitCount, FractionBitCount>::IntegerType;
    static_assert(sizeof(IntegerType) == 4, "This is limited to 32-bit numbers. 64-bit inputs would overflow. Feel free to extend code if needed");

    a.SetRawBits(IntegerType(result >> FractionBitCount));
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator /(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    // Division needs to be performed on the full precision dividend.
    int64_t result = b.GetRawBits() ? (int64_t(a.GetRawBits()) << FractionBitCount) / b.GetRawBits() : INT64_MAX;

    using IntegerType = typename FixedNumber<BaseType, IntegerBitCount, FractionBitCount>::IntegerType;
    static_assert(sizeof(IntegerType) == 4, "This is limited to 32-bit numbers. 64-bit inputs would overflow. Feel free to extend code if needed");

    a.SetRawBits(IntegerType(result));
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator +(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    double b
) noexcept
{
    return a + FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(b);
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator -(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    double b
) noexcept
{
    return a - FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(b);
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator *(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    double b
    ) noexcept
{
    return a * FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(b);
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator /(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> a,
    double b
) noexcept
{
    return a / FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(b);
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator +(
    double a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    return FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(a) + b;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator -(
    double a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    return FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(a) - b;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator *(
    double a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    return FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(a) * b;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount> operator /(
    double a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    return FixedNumber<BaseType, IntegerBitCount, FractionBitCount>(a) / b;
}

//////////////////////////////

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& operator +=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    a.SetRawBits(a.GetRawBits() + b.GetRawBits());
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& operator -=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    a.SetRawBits(a.GetRawBits() - b.GetRawBits());
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& operator *=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
    ) noexcept
{
    a = a * b;
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& operator /=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& a,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> b
) noexcept
{
    a = a / b;
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& operator ++(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& a
) noexcept
{
    a.SetRawBits(a.GetRawBits() + 1);
    return a;
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& operator --(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount>& a
) noexcept
{
    a.SetRawBits(a.GetRawBits() - 1);
    return a;
}

////////////////////////////////////////

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline bool operator ==(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> lhs,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> rhs
) noexcept
{
    return lhs.GetRawBits() == rhs.GetRawBits();
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline bool operator !=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> lhs,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> rhs
) noexcept
{
    return lhs.GetRawBits() != rhs.GetRawBits();
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline bool operator <(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> lhs,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> rhs
) noexcept
{
    return lhs.GetRawBits() < rhs.GetRawBits();
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline bool operator >(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> lhs,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> rhs
) noexcept
{
    return lhs.GetRawBits() > rhs.GetRawBits();
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline bool operator <=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> lhs,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> rhs
) noexcept
{
    return lhs.GetRawBits() <= rhs.GetRawBits();
}

template<typename BaseType, unsigned int IntegerBitCount, unsigned int FractionBitCount>
inline bool operator >=(
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> lhs,
    FixedNumber<BaseType, IntegerBitCount, FractionBitCount> rhs
) noexcept
{
    return lhs.GetRawBits() >= rhs.GetRawBits();
}
