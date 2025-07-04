//-----------------------------------------------------------------------------
//
//  See:
//  https://arxiv.org/abs/2209.05433 FP8 Formats for Deep Learning
//  https://arxiv.org/abs/2206.02915 8-bit Numerical Formats for Deep Neural Networks 2022-10-24
//
//-----------------------------------------------------------------------------

#pragma once

using float8m3e4s1_t = FloatNumber<uint8_t, 3, 4, true, true, false, true>; // No infinity and one NaN representation (S1111.111).

inline float8m3e4s1_t operator +(float8m3e4s1_t a, float8m3e4s1_t b) noexcept { return float(a) + float(b); }
inline float8m3e4s1_t operator -(float8m3e4s1_t a, float8m3e4s1_t b) noexcept { return float(a) - float(b); }
inline float8m3e4s1_t operator *(float8m3e4s1_t a, float8m3e4s1_t b) noexcept { return float(a) * float(b); }
inline float8m3e4s1_t operator /(float8m3e4s1_t a, float8m3e4s1_t b) noexcept { return float(a) / float(b); }
inline float8m3e4s1_t operator +(float8m3e4s1_t a, double b) noexcept { return float(a) + float(b); }
inline float8m3e4s1_t operator -(float8m3e4s1_t a, double b) noexcept { return float(a) - float(b); }
inline float8m3e4s1_t operator *(float8m3e4s1_t a, double b) noexcept { return float(a) * float(b); }
inline float8m3e4s1_t operator /(float8m3e4s1_t a, double b) noexcept { return float(a) / float(b); }
inline float8m3e4s1_t operator +(double a, float8m3e4s1_t b) noexcept { return float(a) + float(b); }
inline float8m3e4s1_t operator -(double a, float8m3e4s1_t b) noexcept { return float(a) - float(b); }
inline float8m3e4s1_t operator *(double a, float8m3e4s1_t b) noexcept { return float(a) * float(b); }
inline float8m3e4s1_t operator /(double a, float8m3e4s1_t b) noexcept { return float(a) / float(b); }
inline float8m3e4s1_t& operator +=(float8m3e4s1_t& a, float8m3e4s1_t b) noexcept { return a = (float(a) + float(b)); }
inline float8m3e4s1_t& operator -=(float8m3e4s1_t& a, float8m3e4s1_t b) noexcept { return a = (float(a) - float(b)); }
inline float8m3e4s1_t& operator *=(float8m3e4s1_t& a, float8m3e4s1_t b) noexcept { return a = (float(a) * float(b)); }
inline float8m3e4s1_t& operator /=(float8m3e4s1_t& a, float8m3e4s1_t b) noexcept { return a = (float(a) / float(b)); }
inline float8m3e4s1_t& operator ++(float8m3e4s1_t& a) noexcept { return a = float(a) + 1; }
inline float8m3e4s1_t& operator --(float8m3e4s1_t& a) noexcept { return a = float(a) - 1; }
inline bool operator==(float8m3e4s1_t lhs, float8m3e4s1_t rhs) noexcept { return float(lhs) == float(rhs); }
inline bool operator!=(float8m3e4s1_t lhs, float8m3e4s1_t rhs) noexcept { return float(lhs) != float(rhs); }
inline bool operator< (float8m3e4s1_t lhs, float8m3e4s1_t rhs) noexcept { return float(lhs) <  float(rhs); }
inline bool operator> (float8m3e4s1_t lhs, float8m3e4s1_t rhs) noexcept { return float(lhs) >  float(rhs); }
inline bool operator<=(float8m3e4s1_t lhs, float8m3e4s1_t rhs) noexcept { return float(lhs) <= float(rhs); }
inline bool operator>=(float8m3e4s1_t lhs, float8m3e4s1_t rhs) noexcept { return float(lhs) >= float(rhs); }
