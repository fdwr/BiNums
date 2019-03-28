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
    static_assert(sizeof(IntegerType) == 4 || sizeof(IntegerType) == 8, "This class can use 32-bit or 64-bit numbers.");

    FixedNumber() = default;
    FixedNumber(const Self&) = default;
    FixedNumber(Self&&) = default;

    FixedNumber(float newValue) noexcept
    {
        SetFloatValue(newValue);
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
        SetFloatValue(newValue);
        return *this;
    }

    IntegerType GetRawBits()
    {
        return value;
    }

    void SetRawBits(int32_t newValue)
    {
        value = newValue;
    }

    void SetFloatValue(float newValue)
    {
        value = static_cast<IntegerType>(newValue * FractionMultiple);
    }

    float GetFloatValue() const noexcept
    {
        return static_cast<float>(IntegerType(value) * FractionInverseMultiple);
    }

    operator float() const noexcept
    {
        return GetFloatValue();
    }

    BaseType value;
};

// TODO: Finish math operations on fixed-size fraction types.
//inline float16m7e8s1_t operator +(float16m7e8s1_t a, float16m7e8s1_t b) noexcept { return float(a) + float(b); }
//inline float16m7e8s1_t operator -(float16m7e8s1_t a, float16m7e8s1_t b) noexcept { return float(a) - float(b); }
//inline float16m7e8s1_t operator *(float16m7e8s1_t a, float16m7e8s1_t b) noexcept { return float(a) * float(b); }
//inline float16m7e8s1_t operator /(float16m7e8s1_t a, float16m7e8s1_t b) noexcept { return float(a) / float(b); }
//inline float16m7e8s1_t operator +(float16m7e8s1_t a, double b) noexcept { return float(a) + float(b); }
//inline float16m7e8s1_t operator -(float16m7e8s1_t a, double b) noexcept { return float(a) - float(b); }
//inline float16m7e8s1_t operator *(float16m7e8s1_t a, double b) noexcept { return float(a) * float(b); }
//inline float16m7e8s1_t operator /(float16m7e8s1_t a, double b) noexcept { return float(a) / float(b); }
//inline float16m7e8s1_t operator +(double a, float16m7e8s1_t b) noexcept { return float(a) + float(b); }
//inline float16m7e8s1_t operator -(double a, float16m7e8s1_t b) noexcept { return float(a) - float(b); }
//inline float16m7e8s1_t operator *(double a, float16m7e8s1_t b) noexcept { return float(a) * float(b); }
//inline float16m7e8s1_t operator /(double a, float16m7e8s1_t b) noexcept { return float(a) / float(b); }
//inline float16m7e8s1_t& operator +=(float16m7e8s1_t& a, float16m7e8s1_t b) noexcept { return a = (float(a) + float(b)); }
//inline float16m7e8s1_t& operator -=(float16m7e8s1_t& a, float16m7e8s1_t b) noexcept { return a = (float(a) - float(b)); }
//inline float16m7e8s1_t& operator *=(float16m7e8s1_t& a, float16m7e8s1_t b) noexcept { return a = (float(a) * float(b)); }
//inline float16m7e8s1_t& operator /=(float16m7e8s1_t& a, float16m7e8s1_t b) noexcept { return a = (float(a) / float(b)); }
//inline float16m7e8s1_t& operator ++(float16m7e8s1_t& a) noexcept { return a = float(a) + 1; }
//inline float16m7e8s1_t& operator --(float16m7e8s1_t& a) noexcept { return a = float(a) + 1; }
//inline bool operator==(float16m7e8s1_t lhs, float16m7e8s1_t rhs) noexcept { return float(lhs) == float(rhs); }
//inline bool operator!=(float16m7e8s1_t lhs, float16m7e8s1_t rhs) noexcept { return float(lhs) != float(rhs); }
//inline bool operator< (float16m7e8s1_t lhs, float16m7e8s1_t rhs) noexcept { return float(lhs) <  float(rhs); }
//inline bool operator> (float16m7e8s1_t lhs, float16m7e8s1_t rhs) noexcept { return float(lhs) >  float(rhs); }
//inline bool operator<=(float16m7e8s1_t lhs, float16m7e8s1_t rhs) noexcept { return float(lhs) <= float(rhs); }
//inline bool operator>=(float16m7e8s1_t lhs, float16m7e8s1_t rhs) noexcept { return float(lhs) >= float(rhs); }
