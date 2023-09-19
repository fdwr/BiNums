//-----------------------------------------------------------------------------
//  General floating point struct with arbitrary bit allocations.
//  Limitations:
//  - no math implementations, just casting from/to standard float types.
//  - binary exponents (no exponents with decimal or hexademical bases).
//  - zero-point bias is only IEEE-style, half the exponent range minus one (e^2 - 1, so 127 for 8-bit exponent).
//  - bit field order in increasing order is always: fraction, exponent, sign. (no odd orderings like exponent, sign, fraction)
//  - hidden one is implicit IEEE-style (some rare float formats explicitly store ones in the fraction part and adjust exponent).
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

#include <stdint.h>
#include "Int24.h"

namespace FloatNumberDefinitions
{
    // Full definition of a floating point representation.
    // Defined outside FloatNumber so it's not dependent on FloatNumber's template parameters.
    template <
        typename BaseIntegerType,
        unsigned int FractionBitCount,
        unsigned int ExponentBitCount,
        bool HasSign,
        bool HasSubnormals,
        bool HasInfinity,
        bool HasNan
    >
    struct Details
    {
        // The warning is bogus, since the shift result is not actually used in such a case.
    #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4293) // '<<': shift count negative or too big, undefined behavior
    #endif

        using baseIntegerType = BaseIntegerType;

        static constexpr BaseIntegerType SafeLeftShift(BaseIntegerType value, uint32_t shift)
        {
            // This weird split shift (1 << (x - 1) << 1) instead of just (1 << x) is to avoid the unfortunate x86 shift wrap around bug -__-.
            return (shift > 0) ? (value << (shift - 1) << 1) : value;
        }

        static constexpr const unsigned int fractionBitCount              = FractionBitCount;
        static constexpr const unsigned int exponentBitCount              = ExponentBitCount;
        static constexpr const bool hasSign                               = HasSign;
        static constexpr const bool hasSubnormals                         = HasSubnormals;
        static constexpr const bool hasInfinity                           = HasInfinity;
        static constexpr const bool hasNan                                = HasNan;

        static constexpr const uint32_t totalBitCount                     = sizeof(BaseIntegerType) * CHAR_BIT;
        static constexpr const uint32_t fractionBitOffset                 = 0;
        static constexpr const uint32_t signBitOffset                     = HasSign ? totalBitCount - 1 : 0; // Sign bit is always at top, if present.
        static constexpr const uint32_t exponentBitOffset                 = FractionBitCount; // Exponent starts immediately after fraction bits.
        static constexpr const int32_t  exponentMin                       = 0;
        static constexpr const int32_t  exponentMax                       = ExponentBitCount ? (1u << ExponentBitCount) - 1 : 0;
        static constexpr const int32_t  exponentBias                      = ExponentBitCount ? (1u << (ExponentBitCount - 1)) - 1 : 0;
        static constexpr const BaseIntegerType zero                       = BaseIntegerType(0);
        static constexpr const BaseIntegerType ulp                        = BaseIntegerType(1); // Unit last place.
        static constexpr const BaseIntegerType signMask                   = (HasSign ? ulp : zero) << signBitOffset;
        static constexpr const BaseIntegerType fractionMask               = SafeLeftShift(ulp, FractionBitCount) - 1;
        static constexpr const BaseIntegerType exponentMask               = SafeLeftShift(ulp, FractionBitCount + ExponentBitCount) - SafeLeftShift(ulp, FractionBitCount);
        static constexpr const BaseIntegerType fractionAndExponentMask    = fractionMask | exponentMask;
        static constexpr const BaseIntegerType maximumLegalBitValue       = (!HasInfinity && !HasNan) ? fractionAndExponentMask // Max value is saturated to all 1's.
                                                                          : ( HasInfinity && !HasNan) ? fractionAndExponentMask // Max value is saturated to all 1's.
                                                                          : (!HasInfinity &&  HasNan) ? fractionAndExponentMask - 1 // NaN is all 1's. So one less than that.
                                                                          : /*HasInfinity && HasNan  */ exponentMask; // Fully saturated exponent, but no fraction bits (which would be NaN).
        static constexpr const BaseIntegerType minimumNanBitValue         = !HasNan        ? 0
                                                                          : HasInfinity    ? exponentMask + 1 // First NaN starts right after infinity
                                                                          : /*!HasInfinity*/ fractionAndExponentMask; // NaN is all 1's.
        static constexpr const BaseIntegerType quietNanMask               = HasNan ? (fractionMask ^ (fractionMask >> 1)) : 0; // Clear all bits below the top one.
    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif
    }; // Details

    using Float8f3e4s1      = Details<uint8_t,  3, 4,   true, true, false, true>;  // No infinity and one NaN representation (S1111.111).
    using Float8f2e5s1      = Details<uint8_t,  2, 5,   true, true, true,  true>;
    using Float16           = Details<uint32_t, 10, 5,  true, true, true,  true>; // https://en.wikipedia.org/wiki/Half-precision_floating-point_format
    using Float32           = Details<uint32_t, 23, 8,  true, true, true,  true>;
    using Float64           = Details<uint64_t, 52, 11, true, true, true,  true>;
    using Float16f10e5s1    = Details<uint16_t, 10, 5,  true, true, true,  true>; // "Brain" float https://en.wikipedia.org/wiki/Bfloat16_floating-point_format
    using Float24f15e8s1    = Details<uint24_t, 15, 8,  true, true, true,  true>; // Pixar PXR24 https://www.openexr.com/documentation/TechnicalIntroduction.pdf, https://en.wikipedia.org/w/index.php?title=Bfloat16_floating-point_format&oldid=1028845625#bfloat16_floating-point_format
    using Float24f16e7s1    = Details<uint24_t, 16, 7,  true, true, true,  true>; // AMD Radeon R300 and R420 https://en.wikipedia.org/wiki/Minifloat, https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter32.html
    #if defined(UINT128MAX) || __SIZEOF_INT128__ //  https://stackoverflow.com/questions/18531782/how-to-know-if-uint128-t-is-defined
        using Float128      = Details<uint128_t, 112, 15, true, true, true>; // Most compilers lack a uint128_t.
    #endif

    // Minihelper shifts left if positive (right if negative).
    template <typename T>
    inline T constexpr LeftRightShift(T t, int32_t shift) noexcept
    {
        return (shift >= 0) ? (t << shift) : (t >> -shift);
    }

    template <typename SourceFloatDefinition, typename TargetFloatDefinition>
    static constexpr typename TargetFloatDefinition::baseIntegerType ConvertRawFloatType(typename SourceFloatDefinition::baseIntegerType sourceValue) noexcept
    {
        // Shift the fraction, exponent, and sign from their respective locations in the float32
        // to the target type.
        // Sature the exponent if greater than can be represented.
        // Flush subnormals to zero if denorms are not supported.

        using Source = SourceFloatDefinition;
        using Target = TargetFloatDefinition;
        using IntermediateType = std::conditional_t<(Source::totalBitCount > Target::totalBitCount), typename Source::baseIntegerType, typename Target::baseIntegerType>;

        if (Target::exponentBitCount == Source::exponentBitCount && Target::hasSign == Source::hasSign)
        {
            // Optimized path can just shift. This applies to bfloat16 <-> IEEE float32.
            IntermediateType const sourceIntermediate = IntermediateType(sourceValue);
            IntermediateType const targetValue = LeftRightShift(sourceIntermediate, int32_t(Target::totalBitCount - Source::totalBitCount));
            return TargetFloatDefinition::baseIntegerType(targetValue);
        }
        else // More complex path.
        {
            // TODO: Consider rounding when converting to smaller fraction bit count, rather than just truncating them.
            int32_t constexpr sourceToTargetShift = int32_t(Target::fractionBitCount - Source::fractionBitCount);
            IntermediateType constexpr exponentAdjustment = IntermediateType(Target::exponentBias - Source::exponentBias) << Target::fractionBitCount;
            IntermediateType const sourceSign = IntermediateType(sourceValue & Source::signMask);
            IntermediateType const targetSign = LeftRightShift(sourceSign, Target::signBitOffset - Source::signBitOffset);
            IntermediateType const sourceFractionAndExponent = IntermediateType(sourceValue & Source::fractionAndExponentMask);
            IntermediateType const unadjustedFractionAndExponent = LeftRightShift(sourceFractionAndExponent, sourceToTargetShift);
            IntermediateType targetFractionAndExponent = unadjustedFractionAndExponent + exponentAdjustment;
            bool constexpr targetHasSmallerExponent = Target::exponentBitCount < Source::exponentBitCount;

            // Preserve NaN when both source and target have the property.
            // If only source or destination has NaN, fall through to saturation below.
            // NaN is defined is having the maximum exponent and a nonzero fraction.
            // So the fraction-and-exponent bit value is greater than the exponent mask alone.
            if (Source::hasNan && Target::hasNan && (sourceFractionAndExponent >= Source::minimumNanBitValue))
            {
                // Preserve the remaining NaN payload, but ensure the quiet bit is set.
                targetFractionAndExponent = (targetFractionAndExponent & Target::fractionMask) | Target::minimumNanBitValue | Target::quietNanMask;
            }
            else if (Source::hasInfinity && Target::hasInfinity && (sourceFractionAndExponent == Source::maximumLegalBitValue))
            {
                // Just set target to infinity, using the largest value that isn't NaN.
                targetFractionAndExponent = Target::maximumLegalBitValue;
            }
            else if (
                (sourceFractionAndExponent == 0)
            ||  (targetHasSmallerExponent && targetFractionAndExponent > unadjustedFractionAndExponent) // Underflow
            ||  (!targetHasSmallerExponent && targetFractionAndExponent < unadjustedFractionAndExponent) // Underflow
            ||  (!Target::hasSubnormals && targetFractionAndExponent <= Target::fractionMask) // Flush subnormals to zero
                )
            {
                targetFractionAndExponent = 0;
            }
            else if (targetFractionAndExponent > Target::maximumLegalBitValue)
            {
                // Saturate to maximal positive value just before NaN.
                targetFractionAndExponent = Target::maximumLegalBitValue;
            }

            IntermediateType targetValue = targetFractionAndExponent | targetSign;
            return TargetFloatDefinition::baseIntegerType(targetValue);
        }
    }

} // namespace FloatNumberDefinitions


// Generic FloatNumber type.
//
// FloatNumber<uint16_t, 10, 5, true, true, true, true> - IEEE float16
// FloatNumber<uint32_t, 23, 8, true, true, true, true> - IEEE float32
// FloatNumber<uint64_t, 52, 11, true, true, true, true> - IEEE float64
// FloatNumber<uint16_t, 10, 6, false, true, true, true> - float with no sign and wider range
// FloatNumber<uint64_t, 48, 16, false, true, true, true> - no sign bit, larger exponent than float64
// 
// TODO: Make atypical cases like no exponent or no fraction also work.
//                              sign  subnm inf
// FloatNumber<uint32_t, 0, 31, true, true, true>
// FloatNumber<uint32_t, 31, 0, true, true, true>
//
// constexpr float testNumbers[] = {0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 65504.0f, -65504.0f, 16777216.0f, -16777216.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::quiet_NaN(), -std::numeric_limits<float>::infinity()};
//
template <
    typename BaseIntegerType, // e.g. uint32_t, uint64_t
    unsigned int FractionBitCount,
    unsigned int ExponentBitCount,
    bool HasSign,
    bool HasSubnormals,
    bool HasInfinity,
    bool HasNan
>
struct FloatNumber
{
    using Self = FloatNumber<BaseIntegerType, FractionBitCount, ExponentBitCount, HasSign, HasSubnormals, HasInfinity, HasNan>;
    using SelfDefinition = FloatNumberDefinitions::Details<BaseIntegerType, FractionBitCount, ExponentBitCount, HasSign, HasSubnormals, HasInfinity, HasNan>;

    BaseIntegerType value;

    FloatNumber() = default;
    FloatNumber(const FloatNumber&) = default;
    FloatNumber(FloatNumber&&) = default;

    constexpr FloatNumber(float floatValue) noexcept
    {
        value = FloatNumberDefinitions::ConvertRawFloatType<FloatNumberDefinitions::Float32, SelfDefinition>(reinterpret_cast<uint32_t&>(floatValue));
    }

    constexpr FloatNumber(double floatValue) noexcept
    {
        value = FloatNumberDefinitions::ConvertRawFloatType<FloatNumberDefinitions::Float64, SelfDefinition>(reinterpret_cast<uint64_t&>(floatValue));
    }

    constexpr FloatNumber& operator =(const FloatNumber&) noexcept = default;

    constexpr inline FloatNumber& operator =(float floatValue) noexcept
    {
        new(this) FloatNumber(floatValue);
        return *this;
    }

    constexpr inline FloatNumber& operator =(double floatValue) noexcept
    {
        new(this) FloatNumber(floatValue);
        return *this;
    }

    constexpr operator float() const noexcept
    {
        float floatValue;
        reinterpret_cast<uint32_t&>(floatValue) = FloatNumberDefinitions::ConvertRawFloatType<SelfDefinition, FloatNumberDefinitions::Float32>(value);
        return floatValue;
    }

    constexpr operator double() const noexcept
    {
        double floatValue;
        reinterpret_cast<uint64_t&>(floatValue) = FloatNumberDefinitions::ConvertRawFloatType<SelfDefinition, FloatNumberDefinitions::Float64>(value);
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
}; // FloatNumber
