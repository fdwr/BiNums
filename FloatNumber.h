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

namespace FloatNumberDetails
{

template <
    typename BaseIntegerType,
    unsigned int FractionBitCount,
    unsigned int ExponentBitCount,
    bool HasSign,
    bool HasSubnormals,
    bool HasInfinity
>
struct FloatDefinition
{
    // The warning is bogus, since the shift result is not actually used in such a case.
    #pragma warning(push)
    #pragma warning(disable : 4293) // '<<': shift count negative or too big, undefined behavior

    using baseIntegerType = BaseIntegerType;

    static constexpr BaseIntegerType SafeLeftShift(BaseIntegerType value, uint32_t shift)
    {
        // This weird split shift (1 << (x - 1) << 1) instead of just (1 << x) is to avoid the unfortunate x86 shift wrap around bug -__-.
        return (shift > 0) ? (value << (shift - 1) << 1) : value;
    }

    static constexpr unsigned int fractionBitCount              = FractionBitCount;
    static constexpr unsigned int exponentBitCount              = ExponentBitCount;
    static constexpr bool hasSubnormals                         = HasSubnormals;
    static constexpr bool hasInfinity                           = HasInfinity;
    static constexpr bool hasSign                               = HasSign;

    static constexpr uint32_t totalBitCount                     = sizeof(BaseIntegerType) * CHAR_BIT;
    static constexpr uint32_t fractionBitOffset                 = 0;
    static constexpr uint32_t signBitOffset                     = HasSign ? totalBitCount - 1 : 0; // Sign bit is always at top, if present.
    static constexpr uint32_t exponentBitOffset                 = FractionBitCount; // Exponent starts immediately after fraction bits.
    static constexpr int32_t  exponentMin                       = 0;
    static constexpr int32_t  exponentMax                       = ExponentBitCount ? (1u << ExponentBitCount) - 1 : 0;
    static constexpr int32_t  exponentBias                      = ExponentBitCount ? (1u << (ExponentBitCount - 1)) - 1 : 0;
    static constexpr BaseIntegerType zero                       = BaseIntegerType(0);
    static constexpr BaseIntegerType ulp                        = BaseIntegerType(1); // Unit last lace.
    static constexpr BaseIntegerType signMask                   = (HasSign ? ulp : zero) << signBitOffset;
    static constexpr BaseIntegerType fractionMask               = SafeLeftShift(ulp, FractionBitCount) - 1;
    static constexpr BaseIntegerType exponentMask               = SafeLeftShift(ulp, FractionBitCount + ExponentBitCount) - SafeLeftShift(ulp, FractionBitCount);
    static constexpr BaseIntegerType fractionAndExponentMask    = fractionMask | exponentMask;
    static constexpr BaseIntegerType maximumLegalValue          = HasInfinity ? exponentMask : fractionAndExponentMask; // Clamp to positive infinity and max bit value.
    #pragma warning(pop)
};

// Minihelper shifts left if positive (right if negative).
template <typename T>
T constexpr LeftRightShift(T t, int32_t shift) noexcept
{
    return (shift >= 0) ? (t << shift) : (t >> -shift);
}

template <typename SourceFloatDefinition, typename TargetFloatDefinition>
static constexpr TargetFloatDefinition::baseIntegerType ConvertRawFloatType(typename SourceFloatDefinition::baseIntegerType sourceValue) noexcept
{
    // Shift the fraction, exponent, and sign from their respective locations in the float32
    // to the target type.
    // Sature the exponent if greater than can be represented.
    // Flush subnormals to zero if denorms are not supported.

    using Source = SourceFloatDefinition;
    using Target = TargetFloatDefinition;
    using IntermediateType = std::conditional_t<(Source::totalBitCount > Target::totalBitCount), Source::baseIntegerType, Target::baseIntegerType>;

    if (Target::exponentBitCount == Source::exponentBitCount && Target::hasSign == Source::hasSign)
    {
        // Optimized path can just shift. This applies to bfloat16 <-> IEEE float32.
        IntermediateType const sourceIntermediate = IntermediateType(sourceValue);
        IntermediateType const targetValue = LeftRightShift(sourceIntermediate, Target::totalBitCount - Source::totalBitCount);
        return TargetFloatDefinition::baseIntegerType(targetValue);
    }
    else // More complex path.
    {
        // TODO: Consider rounding when converting to smaller fraction bit count, rather than just truncating them.
        IntermediateType constexpr exponentAdjustment = IntermediateType(Target::exponentBias - Source::exponentBias) << Target::fractionBitCount;
        IntermediateType const sourceSign = IntermediateType(sourceValue & Source::signMask);
        IntermediateType const targetSign = LeftRightShift(sourceSign, Target::signBitOffset - Source::signBitOffset);
        IntermediateType const sourceFractionAndExponent = IntermediateType(sourceValue & Source::fractionAndExponentMask);
        IntermediateType const unadjustedFractionAndExponent = LeftRightShift(sourceFractionAndExponent, Target::fractionBitCount - Source::fractionBitCount);
        IntermediateType targetFractionAndExponent = unadjustedFractionAndExponent + exponentAdjustment;
        bool constexpr targetHasSmallerExponent = Target::exponentBitCount < Source::exponentBitCount;

        if ((targetHasSmallerExponent  && targetFractionAndExponent > unadjustedFractionAndExponent) // Underflow
        ||  (!targetHasSmallerExponent && targetFractionAndExponent < unadjustedFractionAndExponent) // Underflow
        ||  (!Target::hasSubnormals && targetFractionAndExponent <= Target::fractionMask)) // Flush subnormals to zero
        {
            targetFractionAndExponent = 0;
        }
        else if (targetFractionAndExponent > Target::maximumLegalValue)
        {
            // Check positive saturation to avoid NaN (if infinity supported) or overflow (if no inifinity).
            targetFractionAndExponent = Target::maximumLegalValue;
        }

        IntermediateType targetValue = targetFractionAndExponent | targetSign;
        return TargetFloatDefinition::baseIntegerType(targetValue);
    }
}

} // namespace FloatNumberDetails

template <
    typename BaseIntegerType, // e.g. uint32_t, uint64_t
    unsigned int FractionBitCount,
    unsigned int ExponentBitCount,
    bool HasSign,
    bool HasSubnormals,
    bool HasInfinity
>
struct FloatNumber
{
    BaseIntegerType value;

    using Self = FloatNumber<BaseIntegerType, FractionBitCount, ExponentBitCount, HasSign, HasSubnormals, HasInfinity>;
    using FloatDefinition = FloatNumberDetails::FloatDefinition<BaseIntegerType, FractionBitCount, ExponentBitCount, HasSign, HasSubnormals, HasInfinity>;
    using Float32Definition = FloatNumberDetails::FloatDefinition<uint32_t, 23, 8, true, true, true>;
    using Float64Definition = FloatNumberDetails::FloatDefinition<uint64_t, 52, 8, true, true, true>;

    FloatNumber() = default;
    FloatNumber(const FloatNumber&) = default;
    FloatNumber(FloatNumber&&) = default;

    constexpr FloatNumber(float floatValue) noexcept
    {
        value = FloatNumberDetails::ConvertRawFloatType<Float32Definition, FloatDefinition>(reinterpret_cast<uint32_t&>(floatValue));
    }

    constexpr FloatNumber& operator =(const FloatNumber&) noexcept = default;

    constexpr inline FloatNumber& operator =(float floatValue) noexcept
    {
        new(this) FloatNumber(floatValue);
        return *this;
    }

    constexpr operator float() const noexcept
    {
        float floatValue;
        reinterpret_cast<uint32_t&>(floatValue) = FloatNumberDetails::ConvertRawFloatType<FloatDefinition, Float32Definition>(value);
        return floatValue;
    }

    constexpr BaseIntegerType GetRawBits() const noexcept
    {
        return value;
    }

    constexpr void SetRawBits(BaseIntegerType newValue) noexcept
    {
        value = newValue;
    }

    // constexpr float testNumbers[] = {0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 65504.0f, -65504.0f, 16777216.0f, -16777216.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::quiet_NaN(), -std::numeric_limits<float>::infinity()};

    // FloatNumber<uint16_t, 10, 5, true, true, true> - IEEE float16
    // FloatNumber<uint32_t, 23, 8, true, true, true> - IEEE float32
    // FloatNumber<uint64_t, 52, 11, true, true, true> - IEEE float64
    // FloatNumber<uint16_t, 10, 6, false, true, true> - float with no sign and wider range
    // FloatNumber<uint64_t, 48, 16, false, true, true> - no sign bit, larger exponent than float64
    // 
    // TODO: Make atypical cases like no exponent or fraction also work.
    // FloatNumber<uint32_t, 0, 31, true, true, true>
    // FloatNumber<uint32_t, 31, 0, true, true, true>
};
