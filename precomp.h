#pragma once
#ifndef PCH_H
#define PCH_H

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <string_view>
#include <cassert>
#include <vector>
#include "Half.h"
#include "Int24.h"
#include "FixedNumber.h"
#include "FloatNumber.h"
#include "Float16m7e8s1.h"

using float32_t = float;
using float64_t = double;
using float16_t = half_float::half;
using bfloat16_t = float16m7e8s1_t;

static_assert(sizeof(float32_t) == 4);
static_assert(sizeof(float64_t) == 8);
static_assert(sizeof(float16_t) == 2);
static_assert(sizeof(bfloat16_t) == 2);

#ifndef _MSC_VER
#define _In_z_
#define _Inout_
#define _Out_
#define _Null_terminated_
#endif

#endif //PCH_H
