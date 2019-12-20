// BiNums, see binary numbers

#include "precomp.h"

////////////////////////////////////////////////////////////////////////////////
// Generic functions/classes.

struct Range
{
    uint32_t begin = 0;
    uint32_t end = 0;
};

template <typename T>
class Span
{
    T* begin_ = nullptr;
    T* end_ = nullptr;

public:
    Span() = default;
    Span(const Span&) = default;
    Span(Span&&) = default;

    Span(T* p, size_t s) : begin_(p), end_(begin_ + s) {}
    Span(T* b, T* e) : begin_(b), end_(e) {}

    const T* data() const noexcept { return begin_; }
    T* data() noexcept { return begin_; }
    size_t size() const noexcept { return end_ - begin_; }
    T* begin() const noexcept { return begin_; }
    T* end() const noexcept { return end_; }
    T* begin() noexcept { return begin_; }
    T* end() noexcept { return end_; }
    T& operator [](size_t index) noexcept { return begin_[index]; }
    const T& operator [](size_t index) const noexcept { return begin_[index]; }
    T& front() noexcept { return *begin_; }
    const T& front() const noexcept { return *begin_; }
    T& back() noexcept { return *(end_ - 1); }
    const T& back() const noexcept { return *(end_ - 1); }
    bool empty() const noexcept { return begin_ == end_; }

    void PopFront() { ++begin_; }
    void PopBack() { ++end_; }
};

template <typename Enum>
bool ComparedMaskedFlags(Enum e, Enum mask, Enum value)
{
    static_assert(sizeof(Enum) == sizeof(uint32_t)); // Doesn't work with 64-bit enums.
    return (static_cast<uint32_t>(e) & static_cast<uint32_t>(mask)) == static_cast<uint32_t>(value);
}

template <typename Enum>
Enum SetFlags(Enum e, Enum clear, Enum set)
{
    return static_cast<Enum>((static_cast<uint32_t>(e) & ~static_cast<uint32_t>(clear)) | static_cast<uint32_t>(set));
}

template <typename T, typename O>
T& CastReferenceAs(O& o)
{
    return reinterpret_cast<T&>(o);
}

template <typename T, typename O>
const T& CastReferenceAs(O const& o)
{
    return reinterpret_cast<const T&>(o);
}

std::string GetFormattedMessage(char const* formatString, ...)
{
    std::string formattedMessage;

    va_list argList;
    va_start(argList, formatString);
    char buffer[1000];
    buffer[0] = 0;

    vsnprintf(buffer, std::size(buffer), formatString, argList);
    formattedMessage.assign(buffer);
    return formattedMessage;
}

void AppendFormattedMessage(std::string& s, char const* formatString, ...)
{
    std::string formattedMessage;

    va_list argList;
    va_start(argList, formatString);
    char buffer[1000];
    buffer[0] = 0;

    vsnprintf(buffer, std::size(buffer), formatString, argList);
    s.append(buffer);
}

using namespace std::literals::string_view_literals;

////////////////////////////////////////////////////////////////////////////////

using Fixed12_12 = FixedNumber<int24_t, 12, 12>;
using Fixed16_16 = FixedNumber<int32_t, 16, 16>;
using Fixed8_24  = FixedNumber<int32_t, 8, 24>;

union NumberUnion
{
    uint8_t         buffer[8];
    uint8_t         ui8;
    uint16_t        ui16;
    uint32_t        ui32;
    uint64_t        ui64;
    int8_t          i8;
    int16_t         i16;
    int32_t         i32;
    int64_t         i64;
    float16_t       f16;
    float16m7e8s1_t f16m7e8s1;
    float32_t       f32;
    float64_t       f64;
    Fixed12_12      f12_12;
    Fixed16_16      f16_16;
    Fixed8_24       f8_24;

    NumberUnion() : ui64(0) {} // No initialization.
};

enum class ElementType : uint32_t
{
    Undefined = 0,
    Float32 = 1,    // starting from bit 0 - mantissa:23 exponent:8 sign:1
    Uint8 = 2,
    Int8 = 3,
    Uint16 = 4,
    Int16 = 5,
    Int32 = 6,
    Int64 = 7,
    StringChar8 = 8,
    Bool8 = 9,
    Float16m10e5s1 = 10, // mantissa:10 exponent:5 sign:1
    Float16 = Float16m10e5s1,
    Float64 = 11,   // mantissa:52 exponent:11 sign:1
    Uint32 = 12,
    Uint64 = 13,
    Complex64 = 14,
    Complex128 = 15,
    Float16m7e8s1 = 16,   // mantissa:7 exponent:8 sign:1
    Bfloat16 = Float16m7e8s1,
    Fixed24f12i12 = 17, // TODO: Make naming more consistent. Fixed24f12i12 vs Fixed12_12 vs f12_12
    Fixed32f16i16 = 18,
    Fixed32f24i8 = 19,
    Total = 20,
};

enum class NumericOperationType : uint32_t
{
    None,
    Add,
    Subtract,
    Multiply,
    Divide,
    Dot,
    Total
};

enum class NumericPrintingFlags : uint32_t
{
    Default = 0,

    ShowDataMask = 3,
    ShowRawHex = 0,
    ShowRawBinary = 1,
    ShowRawDecimal = 2,
    ShowRawOctal = 3,

    ShowFloatMask = 4,
    ShowFloatDecimal = 0,
    ShowFloatHex = 4,

    ShowRawFieldsMask = 8,
    HideRawFields = 0,
    ShowRawFields = 8,

    ShowRawBinaryFields = ShowRawBinary | ShowRawFields,
};

struct NumberUnionAndType
{
    NumberUnion numberUnion;
    ElementType elementType;
    NumericPrintingFlags printingFlags;
};

struct NumericOperationAndRange
{
    NumericOperationType numericOperationType;
    Range range;
    ElementType outputElementType;
};

struct NumberSubstructure
{
    Range fraction;
    Range integer;
    Range exponent;
    Range sign;

    // e.g. {0,30, 0,0, 30,31, 31,32} float32
};

////////////////////////////////////////////////////////////////////////////////

const char* g_elementTypeNames[] =
{
    "undefined",    // Undefined = 0,
    "float32",      // Float32 = 1,
    "uint8",        // Uint8 = 2,
    "int8",         // Int8 = 3,
    "uint16",       // Uint16 = 4,
    "int16",        // Int16 = 5,
    "int32",        // Int32 = 6,
    "int64",        // Int64 = 7,
    "string8",      // StringChar8 = 8,
    "bool8",        // Bool = 9,
    "float16",      // Float16 = 10,
    "float64",      // Float64 = 11,
    "uint32",       // Uint32 = 12,
    "uint64",       // Uint64 = 13,
    "complex64",    // Complex64 = 14,
    "complex128",   // Complex128 = 15,
    "bfloat16",     // Float16m7e8s1 = 16,
    "fixed12_12",   // Fixed24f12i12 = 17,
    "fixed16_16",   // Fixed32f16i16 = 18,
    "fixed8_24",    // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_elementTypeNames) == 20);

const char* g_numericOperationTypeNames[] =
{
    "nop",
    "add",
    "subtract",
    "multiply",
    "divide",
    "dot",
};
static_assert(int(NumericOperationType::Total) == 6 && std::size(g_numericOperationTypeNames) == 6);

const static uint8_t g_byteSizeOfElementType[] =
{
    0,  // Undefined = 0,
    4,  // Float32 = 1,
    1,  // Uint8 = 2,
    1,  // Int8 = 3,
    2,  // Uint16 = 4,
    2,  // Int16 = 5,
    4,  // Int32 = 6,
    8,  // Int64 = 7,
    0,  // sizeof(TypedBufferStringElement), // StringChar8 = 8,
    1,  // Bool = 9,
    2,  // Float16 = 10,
    8,  // Float64 = 11,
    4,  // Uint32 = 12,
    8,  // Uint64 = 13,
    8,  // Complex64 = 14,
    16, // Complex128 = 15,
    2,  // Float16m7e8s1 = 16,
    3,  // Fixed24f12i12 = 17,
    4,  // Fixed32f16i16 = 18,
    4,  // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_byteSizeOfElementType) == 20);

const static uint8_t g_isFractionalElementType[] =
{
    false, // Undefined = 0,
    true , // Float32 = 1,
    false, // Uint8 = 2,
    false, // Int8 = 3,
    false, // Uint16 = 4,
    false, // Int16 = 5,
    false, // Int32 = 6,
    false, // Int64 = 7,
    false, // sizeof(TypedBufferStringElement), // StringChar8 = 8,
    false, // Bool = 9,
    true , // Float16 = 10,
    true , // Float64 = 11,
    false, // Uint32 = 12,
    false, // Uint64 = 13,
    true , // Complex64 = 14,
    true , // Complex128 = 15,
    true , // Float16m7e8s1 = 16,
    true,  // Fixed24f12i12 = 17,
    true,  // Fixed32f16i16 = 18,
    true,  // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_isFractionalElementType) == 20);

const static uint8_t g_isSignedElementType[] =
{
    false, // Undefined = 0,
    true , // Float32 = 1,
    false, // Uint8 = 2,
    true , // Int8 = 3,
    false, // Uint16 = 4,
    true , // Int16 = 5,
    true , // Int32 = 6,
    true , // Int64 = 7,
    false, // sizeof(TypedBufferStringElement), // StringChar8 = 8,
    false, // Bool = 9,
    true , // Float16 = 10,
    true , // Float64 = 11,
    false, // Uint32 = 12,
    false, // Uint64 = 13,
    true , // Complex64 = 14,
    true , // Complex128 = 15,
    true , // Float16m7e8s1 = 16,
    true , // Fixed24f12i12 = 17,
    true , // Fixed32f16i16 = 18,
    true , // Fixed32f24i8 = 19,
};
static_assert(int(ElementType::Total) == 20 && std::size(g_isSignedElementType) == 20);

// ElementType enum reordered by priority of promotion rules.
enum class ElementTypePriority : uint32_t
{
    Undefined,
    Bool8,
    StringChar8,
    Uint8,
    Int8,
    Uint16,
    Int16,
    Uint32,
    Int32,
    Uint64,
    Int64,
    Float16m10e5s1,
    Float16 = Float16m10e5s1,
    Float32,
    Float64,
    Complex64,
    Complex128,
    Float16m7e8s1,
    Bfloat16 = Float16m7e8s1,
    Fixed32f24i8,
    Fixed24f12i12,
    Fixed32f16i16,
    Total,
};
static_assert(size_t(ElementType::Total) == size_t(ElementTypePriority::Total));

const ElementTypePriority g_elementTypePriorityTable[] = // The index is an ElementType enum.
{
    /* Undefined = 0       */ ElementTypePriority::Undefined,
    /* Float32 = 1         */ ElementTypePriority::Float32,
    /* Uint8 = 2           */ ElementTypePriority::Uint8,
    /* Int8 = 3            */ ElementTypePriority::Int8,
    /* Uint16 = 4          */ ElementTypePriority::Uint16,
    /* Int16 = 5           */ ElementTypePriority::Int16,
    /* Int32 = 6           */ ElementTypePriority::Int32,
    /* Int64 = 7           */ ElementTypePriority::Int64,
    /* StringChar8 = 8     */ ElementTypePriority::StringChar8,
    /* Bool8 = 9           */ ElementTypePriority::Bool8,
    /* Float16m10e5s1 = 10 */ ElementTypePriority::Float16m10e5s1,
    /* Float64 = 11        */ ElementTypePriority::Float64,
    /* Uint32 = 12         */ ElementTypePriority::Uint32,
    /* Uint64 = 13         */ ElementTypePriority::Uint64,
    /* Complex64 = 14      */ ElementTypePriority::Complex64,
    /* Complex128 = 15     */ ElementTypePriority::Complex128,
    /* Float16m7e8s1 = 16  */ ElementTypePriority::Float16m7e8s1,
    /* Fixed24f12i12 = 17  */ ElementTypePriority::Fixed24f12i12,
    /* Fixed32f16i16 = 18  */ ElementTypePriority::Fixed32f16i16,
    /* Fixed32f24i8 = 19   */ ElementTypePriority::Fixed32f24i8,
};
static_assert(std::size(g_elementTypePriorityTable) == size_t(ElementType::Total));

const NumberSubstructure g_elementTypeSubstructures[] = // The index is an ElementType enum.
{
    //                       fraction, integer, exponent, sign
    /* Undefined = 0       */ {{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}},
    /* Float32 = 1         */ {{ 0,23},{ 0, 0},{23,31},{31,32}},
    /* Uint8 = 2           */ {{ 0, 0},{ 0, 8},{ 0, 0},{ 0, 0}},
    /* Int8 = 3            */ {{ 0, 0},{ 0, 7},{ 0, 0},{ 7, 8}},
    /* Uint16 = 4          */ {{ 0, 0},{ 0,16},{ 0, 0},{ 0, 0}},
    /* Int16 = 5           */ {{ 0, 0},{ 0,15},{ 0, 0},{15,16}},
    /* Int32 = 6           */ {{ 0, 0},{ 0,31},{ 0, 0},{31,32}},
    /* Int64 = 7           */ {{ 0, 0},{ 0,63},{ 0, 0},{63,64}},
    /* StringChar8 = 8     */ {{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}},
    /* Bool8 = 9           */ {{ 0, 0},{ 0, 8},{ 0, 0},{ 0, 0}},
    /* Float16m10e5s1 = 10 */ {{ 0,10},{ 0, 0},{10,15},{15,16}},
    /* Float64 = 11        */ {{ 0,52},{ 0, 0},{52,63},{63,64}},
    /* Uint32 = 12         */ {{ 0, 0},{ 0,32},{ 0, 0},{ 0, 0}},
    /* Uint64 = 13         */ {{ 0, 0},{ 0,64},{ 0, 0},{ 0, 0}},
    /* Complex64 = 14      */ {{ 0,23},{ 0, 0},{23,31},{31,32}},
    /* Complex128 = 15     */ {{ 0,52},{ 0, 0},{52,63},{63,64}},
    /* Float16m7e8s1 = 16  */ {{ 0, 7},{ 0, 0},{ 7,15},{15,16}},
    /* Fixed24f12i12 = 17  */ {{ 0,12},{12,24},{ 0, 0},{ 0, 0}},
    /* Fixed32f16i16 = 18  */ {{ 0,16},{16,32},{ 0, 0},{ 0, 0}},
    /* Fixed32f24i8 = 19   */ {{ 0, 8},{ 8,32},{ 0, 0},{ 0, 0}},
};
static_assert(std::size(g_elementTypeSubstructures) == size_t(ElementType::Total));

uint32_t GetSizeOfTypeInBytes(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_byteSizeOfElementType[index < std::size(g_byteSizeOfElementType) ? index : 0];
}

uint32_t GetSizeOfTypeInBits(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_byteSizeOfElementType[index < std::size(g_byteSizeOfElementType) ? index : 0] * 8;
}

std::string_view GetTypeNameFromElementType(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_elementTypeNames[index < std::size(g_elementTypeNames) ? index : 0];
}

bool IsFractionalElementType(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_isFractionalElementType[index < std::size(g_isFractionalElementType) ? index : 0];
}

bool IsSignedElementType(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_isSignedElementType[index < std::size(g_isSignedElementType) ? index : 0];
}

NumberSubstructure const& GetElementTypeSubstructure(ElementType dataType) noexcept
{
    size_t index = static_cast<size_t>(dataType);
    return g_elementTypeSubstructures[index < std::size(g_elementTypeSubstructures) ? index : 0];
}

// Read data type and cast to double.
// The caller passes a data pointer of the given type.
/*static*/ double ReadToDouble(ElementType dataType, const void* data)
{
    double value = 0;
    switch (dataType)
    {
    case ElementType::Float32:          value = *reinterpret_cast<const float*>(data);              break;
    case ElementType::Uint8:            value = *reinterpret_cast<const uint8_t*>(data);            break;
    case ElementType::Int8:             value = *reinterpret_cast<const int8_t*>(data);             break;
    case ElementType::Uint16:           value = *reinterpret_cast<const uint16_t*>(data);           break;
    case ElementType::Int16:            value = *reinterpret_cast<const int16_t*>(data);            break;
    case ElementType::Int32:            value = *reinterpret_cast<const int32_t*>(data);            break;
    case ElementType::Int64:            value = double(*reinterpret_cast<const int64_t*>(data));    break;
    case ElementType::StringChar8:      value = 0; /* no numeric value for strings */               break;
    case ElementType::Bool8:            value = *reinterpret_cast<const bool*>(data);               break;
    case ElementType::Float16:          value = *reinterpret_cast<const float16_t*>(data);          break;
    case ElementType::Bfloat16:         value = *reinterpret_cast<const bfloat16_t*>(data);         break;
    case ElementType::Float64:          value = *reinterpret_cast<const double*>(data);             break;
    case ElementType::Uint32:           value = *reinterpret_cast<const uint32_t*>(data);           break;
    case ElementType::Uint64:           value = double(*reinterpret_cast<const uint64_t*>(data));   break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    value = *reinterpret_cast<const Fixed12_12*>(data);         break;
    case ElementType::Fixed32f16i16:    value = *reinterpret_cast<const Fixed16_16*>(data);         break;
    case ElementType::Fixed32f24i8:     value = *reinterpret_cast<const Fixed8_24*>(data);          break;
    }

    return value;
}

// Read data type and cast to int64.
// The caller passes a data pointer of the given type.
/*static*/ int64_t ReadToInt64(ElementType dataType, const void* data)
{
    int64_t value = 0;
    switch (dataType)
    {
    case ElementType::Float32:          value = int64_t(*reinterpret_cast<const float*>(data));         break;
    case ElementType::Uint8:            value = int64_t(*reinterpret_cast<const uint8_t*>(data));       break;
    case ElementType::Int8:             value = int64_t(*reinterpret_cast<const int8_t*>(data));        break;
    case ElementType::Uint16:           value = int64_t(*reinterpret_cast<const uint16_t*>(data));      break;
    case ElementType::Int16:            value = int64_t(*reinterpret_cast<const int16_t*>(data));       break;
    case ElementType::Int32:            value = int64_t(*reinterpret_cast<const int32_t*>(data));       break;
    case ElementType::Int64:            value = int64_t(*reinterpret_cast<const int64_t*>(data));       break;
    case ElementType::StringChar8:      value = int64_t(0); /* no numeric value for strings */          break;
    case ElementType::Bool8:            value = int64_t(*reinterpret_cast<const bool*>(data));          break;
    case ElementType::Float16:          value = int64_t(*reinterpret_cast<const float16_t*>(data));     break;
    case ElementType::Bfloat16:         value = int64_t(*reinterpret_cast<const bfloat16_t*>(data));    break;
    case ElementType::Float64:          value = int64_t(*reinterpret_cast<const double*>(data));        break;
    case ElementType::Uint32:           value = int64_t(*reinterpret_cast<const uint32_t*>(data));      break;
    case ElementType::Uint64:           value = int64_t(*reinterpret_cast<const uint64_t*>(data));      break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    value = int64_t(*reinterpret_cast<const Fixed12_12*>(data));    break;
    case ElementType::Fixed32f16i16:    value = int64_t(*reinterpret_cast<const Fixed16_16*>(data));    break;
    case ElementType::Fixed32f24i8:;    value = int64_t(*reinterpret_cast<const Fixed8_24*>(data));     break;
    }

    return value;
}

// Read the raw bits, returned as int64.
// Useful for Unit Last Place comparisons.
/*static*/ int64_t ReadRawBitValue(ElementType dataType, const void* data)
{
    int64_t value = 0;
    switch (dataType)
    {
    case ElementType::Float32:          value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Uint8:            value = int64_t(*reinterpret_cast<const uint8_t*>(data));   break;
    case ElementType::Int8:             value = int64_t(*reinterpret_cast<const int8_t*>(data));    break;
    case ElementType::Uint16:           value = int64_t(*reinterpret_cast<const uint16_t*>(data));  break;
    case ElementType::Int16:            value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Int32:            value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Int64:            value = int64_t(*reinterpret_cast<const int64_t*>(data));   break;
    case ElementType::StringChar8:      value = int64_t(0); /* no numeric value for strings */      break;
    case ElementType::Bool8:            value = int64_t(*reinterpret_cast<const bool*>(data));      break;
    case ElementType::Float16:          value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Bfloat16:         value = int64_t(*reinterpret_cast<const int16_t*>(data));   break;
    case ElementType::Float64:          value = int64_t(*reinterpret_cast<const int64_t*>(data));   break;
    case ElementType::Uint32:           value = int64_t(*reinterpret_cast<const uint32_t*>(data));  break;
    case ElementType::Uint64:           value = int64_t(*reinterpret_cast<const uint64_t*>(data));  break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    value = int64_t(*reinterpret_cast<const int24_t*>(data));   break;
    case ElementType::Fixed32f16i16:    value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    case ElementType::Fixed32f24i8:;    value = int64_t(*reinterpret_cast<const int32_t*>(data));   break;
    }

    return value;
}

// The caller passes a data pointer of the given type.
void WriteFromDouble(ElementType dataType, double value, /*out*/ void* data)
{
    switch (dataType)
    {
    case ElementType::Float32:          *reinterpret_cast<float*>(data) = float(value);             break;
    case ElementType::Uint8:            *reinterpret_cast<uint8_t*>(data) = uint8_t(value);         break;
    case ElementType::Int8:             *reinterpret_cast<int8_t*>(data) = int8_t(value);           break;
    case ElementType::Uint16:           *reinterpret_cast<uint16_t*>(data) = uint16_t(value);       break;
    case ElementType::Int16:            *reinterpret_cast<int16_t*>(data) = int16_t(value);         break;
    case ElementType::Int32:            *reinterpret_cast<int32_t*>(data) = int32_t(value);         break;
    case ElementType::Int64:            *reinterpret_cast<int64_t*>(data) = int64_t(value);         break;
    case ElementType::StringChar8:      /* no change value for strings */                           break;
    case ElementType::Bool8:            *reinterpret_cast<bool*>(data) = bool(value);               break;
    case ElementType::Float16:          *reinterpret_cast<uint16_t*>(data) = half_float::detail::float2half<std::round_to_nearest, float>(float(value)); break;
    case ElementType::Bfloat16:         *reinterpret_cast<bfloat16_t*>(data) = bfloat16_t(float(value)); break;
    case ElementType::Float64:          *reinterpret_cast<double*>(data) = value;                   break;
    case ElementType::Uint32:           *reinterpret_cast<uint32_t*>(data) = uint32_t(value);       break;
    case ElementType::Uint64:           *reinterpret_cast<uint64_t*>(data) = uint64_t(value);       break;
    case ElementType::Complex64:        throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:       throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12:    *reinterpret_cast<Fixed12_12*>(data) = float(value);        break;
    case ElementType::Fixed32f16i16:    *reinterpret_cast<Fixed16_16*>(data) = float(value);        break;
    case ElementType::Fixed32f24i8:;    *reinterpret_cast<Fixed8_24*>(data) = float(value);         break;
    }

    // Use half_float::detail::float2half explicitly rather than the half constructor.
    // Otherwise values do not round-trip as expected.
    //
    // e.g. If you print float16 0x2C29, you get 0.0650024, but if you try to parse
    // 0.0650024, you get 0x2C28 instead. Then printing 0x2C28 shows 0.0649414,
    // but trying to parse 0.0649414 returns 0x2C27. Rounding to nearest fixes this.
}

// The caller passes a data pointer of the given type.
void WriteFromInt64(ElementType dataType, int64_t value, /*out*/ void* data)
{
    switch (dataType)
    {
    case ElementType::Float32:       *reinterpret_cast<float*>(data) = float(value);                    break;
    case ElementType::Uint8:         *reinterpret_cast<uint8_t*>(data) = uint8_t(value);                break;
    case ElementType::Int8:          *reinterpret_cast<int8_t*>(data) = int8_t(value);                  break;
    case ElementType::Uint16:        *reinterpret_cast<uint16_t*>(data) = uint16_t(value);              break;
    case ElementType::Int16:         *reinterpret_cast<int16_t*>(data) = int16_t(value);                break;
    case ElementType::Int32:         *reinterpret_cast<int32_t*>(data) = int32_t(value);                break;
    case ElementType::Int64:         *reinterpret_cast<int64_t*>(data) = int64_t(value);                break;
    case ElementType::StringChar8:   /* no change value for strings */                                  break;
    case ElementType::Bool8:         *reinterpret_cast<bool*>(data) = bool(value);                      break;
    case ElementType::Float16:       *reinterpret_cast<float16_t*>(data) = float16_t(float(value));     break;
    case ElementType::Bfloat16:      *reinterpret_cast<bfloat16_t*>(data) = bfloat16_t(float(value));   break;
    case ElementType::Float64:       *reinterpret_cast<double*>(data) = double(value);                  break;
    case ElementType::Uint32:        *reinterpret_cast<uint32_t*>(data) = uint32_t(value);              break;
    case ElementType::Uint64:        *reinterpret_cast<uint64_t*>(data) = uint64_t(value);              break;
    case ElementType::Complex64:     throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:    throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12: *reinterpret_cast<Fixed12_12*>(data) = float(value);               break;
    case ElementType::Fixed32f16i16: *reinterpret_cast<Fixed16_16*>(data) = float(value);               break;
    case ElementType::Fixed32f24i8:  *reinterpret_cast<Fixed8_24*>(data) = float(value);                break;
    }
}

// Just copy a single element from the input to output.
void ReadWriteElementType(ElementType dataType, void const* inputData, /*out*/ void* outputData)
{
    switch (dataType)
    {
    case ElementType::Float32:       *reinterpret_cast<uint32_t*>(outputData)   = *reinterpret_cast<const uint32_t*>(inputData);    break;
    case ElementType::Uint8:         *reinterpret_cast<uint8_t*>(outputData)    = *reinterpret_cast<const uint8_t*>(inputData);     break;
    case ElementType::Int8:          *reinterpret_cast<uint8_t*>(outputData)    = *reinterpret_cast<const uint8_t*>(inputData);     break;
    case ElementType::Uint16:        *reinterpret_cast<uint16_t*>(outputData)   = *reinterpret_cast<const uint16_t*>(inputData);    break;
    case ElementType::Int16:         *reinterpret_cast<uint16_t*>(outputData)   = *reinterpret_cast<const uint16_t*>(inputData);    break;
    case ElementType::Int32:         *reinterpret_cast<uint32_t*>(outputData)   = *reinterpret_cast<const uint32_t*>(inputData);    break;
    case ElementType::Int64:         *reinterpret_cast<uint64_t*>(outputData)   = *reinterpret_cast<const uint64_t*>(inputData);    break;
    case ElementType::StringChar8:   /* no change value for strings */                                                              break;
    case ElementType::Bool8:         *reinterpret_cast<uint8_t*>(outputData)    = *reinterpret_cast<const uint8_t*>(inputData);     break;
    case ElementType::Float16:       *reinterpret_cast<uint16_t*>(outputData)   = *reinterpret_cast<const uint16_t*>(inputData);    break;
    case ElementType::Bfloat16:      *reinterpret_cast<uint16_t*>(outputData)   = *reinterpret_cast<const uint16_t*>(inputData);    break;
    case ElementType::Float64:       *reinterpret_cast<uint64_t*>(outputData)   = *reinterpret_cast<const uint64_t*>(inputData);    break;
    case ElementType::Uint32:        *reinterpret_cast<uint32_t*>(outputData)   = *reinterpret_cast<const uint32_t*>(inputData);    break;
    case ElementType::Uint64:        *reinterpret_cast<uint64_t*>(outputData)   = *reinterpret_cast<const uint64_t*>(inputData);    break;
    case ElementType::Complex64:     throw std::invalid_argument("Complex64 type is not supported.");
    case ElementType::Complex128:    throw std::invalid_argument("Complex128 type is not supported.");
    case ElementType::Fixed24f12i12: *reinterpret_cast<Fixed12_12*>(outputData) = *reinterpret_cast<const Fixed12_12*>(inputData);  break;
    case ElementType::Fixed32f16i16: *reinterpret_cast<uint32_t*>(outputData)   = *reinterpret_cast<const uint32_t*>(inputData);    break;
    case ElementType::Fixed32f24i8:  *reinterpret_cast<uint32_t*>(outputData)   = *reinterpret_cast<const uint32_t*>(inputData);    break;
    }
}

// Cast copy a single element from the input type to output type.
void ReadWriteElementType(
    ElementType inputDataType,
    ElementType outputDataType,
    void const* inputData,
    /*out*/ void* outputData
)
{
    if (inputDataType == outputDataType)
    {
        ReadWriteElementType(inputDataType, inputData, outputData);
    }
    else if (IsFractionalElementType(inputDataType))
    {
        double value = ReadToDouble(inputDataType, inputData);
        if (IsFractionalElementType(outputDataType))
        {
            WriteFromDouble(outputDataType, value, outputData);
        }
        else
        {
            WriteFromInt64(outputDataType, static_cast<int64_t>(value), outputData);
        }
    }
    else // !IsFractionalElementType(inputDataType)
    {
        int64_t value = ReadToInt64(inputDataType, inputData);
        if (IsFractionalElementType(outputDataType))
        {
            WriteFromDouble(outputDataType, static_cast<double>(value), outputData);
        }
        else
        {
            WriteFromInt64(outputDataType, value, outputData);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

std::string_view GetNumericOperationNameFromNumericOperationType(NumericOperationType numericOperationType) noexcept
{
    size_t index = static_cast<size_t>(numericOperationType);
    return g_numericOperationTypeNames[index < std::size(g_numericOperationTypeNames) ? index : 0];
}

void AppendFormattedRawInteger(
    uint32_t radix, // 2 or 16
    Range bitRange,
    uint64_t value,
    /*inout*/ std::string& stringValue
)
{
    const uint32_t bitOffset = bitRange.begin;
    const uint32_t bitCount = bitRange.end - bitOffset;
    const uint64_t valueMask = (1ui64 << bitCount) - 1;
    value >>= bitOffset;
    value &= valueMask;

    switch (radix)
    {
    case 2: // binary
        {
            stringValue.append("0b"sv);
            uint32_t index = bitCount;
            while (index > 0)
            {
                --index;
                bool isBitSet = value & (uint64_t(1) << index);
                stringValue.push_back('0' + isBitSet);
            }
        }
        break;

    case 10: // decimal
        {
            uint64_t maxValue = (uint64_t(1) << bitCount) - 1;
            uint32_t digitCount = static_cast<uint32_t>(floor(log10(maxValue) + 1));
            AppendFormattedMessage(/*inout*/ stringValue, "%.*lld", digitCount, value);
        }
        break;

    case 16: // hexadecimal
        {
            uint32_t digitCount = (bitCount + 3) / 4u;
            AppendFormattedMessage(/*inout*/ stringValue, "0x%.*llX", digitCount, value);
        }
        break;

    default: // arbitrary radix. Using 'itoa' is dubious with unbounded buffer sizes.
        {
            uint64_t maxValue = (uint64_t(1) << bitCount) - 1;
            uint32_t digitCount = static_cast<uint32_t>(floor((log(maxValue) / log(radix)) + 1));

            if (radix == 8)
            {
                stringValue.append("0o"sv);
            }

            size_t oldSize = stringValue.size();
            size_t newSize = oldSize + digitCount;
            size_t index = newSize;
            stringValue.resize(newSize);

            while (index > oldSize)
            {
                --index;
                uint32_t remainder = static_cast<uint32_t>(value % radix); // Note std::div doesn't support uint64_t.
                value /= radix;
                char c = remainder <= 10 ? ('0' + remainder) : ('A' + remainder - 10);
                stringValue[index] = c;
            }
        }
        break;

    }
}

void AppendFormattedRawInteger(
    std::string_view name,
    uint32_t radix, // 2 or 16
    Range bitRange,
    uint64_t value,
    /*inout*/ std::string& stringValue
)
{
    // Print nothing if there are no bits (such as an exponent field for an integer).
    if (bitRange.begin == bitRange.end)
    {
        return;
    }

    // Separate preceding text with a space.
    if (!stringValue.empty())
    {
        char c = stringValue.back();
        if (c != ' ' && c != '(' && c != '[' && c != '{')
        {
            stringValue.push_back(' ');
        }
    }
    stringValue.append(name);
    stringValue.push_back(':');
    AppendFormattedRawInteger(radix, bitRange, value, /*inout*/ stringValue);
}

void AppendFormattedNumericValue(
    ElementType elementType,
    double floatValue,
    int64_t integerValue,
    NumericPrintingFlags printingFlags,
    /*inout*/ std::string& stringValue
)
{
    // Print the numeric component, per typical human-readable value.

    if (IsFractionalElementType(elementType))
    {
        if (ComparedMaskedFlags(printingFlags, NumericPrintingFlags::ShowFloatMask, NumericPrintingFlags::ShowFloatHex))
        {
            AppendFormattedMessage(/*inout*/ stringValue, "%a", floatValue);
        }
        else // NumericPrintingFlags::ShowDecimalFloat
        {
            AppendFormattedMessage(/*inout*/ stringValue, "%.24g", floatValue);
        }
    }
    else if (IsSignedElementType(elementType))
    {
        AppendFormattedMessage(/*inout*/ stringValue, "%lld", integerValue);
    }
    else // unsigned
    {
        AppendFormattedMessage(/*inout*/ stringValue, "%llu", integerValue);
    }
}

void AppendFormattedNumericValue(
    ElementType elementType,
    int64_t rawBitValue,
    NumericPrintingFlags printingFlags,
    /*inout*/ std::string& stringValue
)
{
    // Print the raw part of the value.

    uint32_t rawDisplayRadix = 16;
    if (ComparedMaskedFlags(printingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawBinary))
    {
        rawDisplayRadix = 2;
    }
    else if (ComparedMaskedFlags(printingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawHex))
    {
        rawDisplayRadix = 16;
    }
    else if (ComparedMaskedFlags(printingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawDecimal))
    {
        rawDisplayRadix = 10;
    }
    else if (ComparedMaskedFlags(printingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawOctal))
    {
        rawDisplayRadix = 8;
    }

    // Print raw data, as binary or hex.
    if (ComparedMaskedFlags(printingFlags, NumericPrintingFlags::ShowRawFieldsMask, NumericPrintingFlags::ShowRawFields))
    {
        NumberSubstructure const& numberSubstructure = GetElementTypeSubstructure(elementType);
        AppendFormattedRawInteger("int"sv, rawDisplayRadix, numberSubstructure.integer, rawBitValue, /*inout*/ stringValue);
        AppendFormattedRawInteger("frac"sv, rawDisplayRadix, numberSubstructure.fraction, rawBitValue, /*inout*/ stringValue);
        AppendFormattedRawInteger("exp"sv, rawDisplayRadix, numberSubstructure.exponent, rawBitValue, /*inout*/ stringValue);
        AppendFormattedRawInteger("sign"sv, rawDisplayRadix, numberSubstructure.sign, rawBitValue, /*inout*/ stringValue);
    }
    else
    {
        const uint32_t sizeOfTypeInBits = GetSizeOfTypeInBits(elementType);
        AppendFormattedRawInteger(rawDisplayRadix, { 0, sizeOfTypeInBits }, rawBitValue, /*inout*/ stringValue);
    }
}

std::string GetFormattedNumericValue(
    ElementType elementType,
    const void* binaryData,
    std::string_view leftFlank,
    std::string_view rightFlank,
    NumericPrintingFlags valueFlags
)
{
    const int64_t rawBitValue = ReadRawBitValue(elementType, binaryData);
    const double floatValue = ReadToDouble(elementType, binaryData);

    const char* elementTypeName = GetTypeNameFromElementType(elementType).data();

    // Print formatted string. e.g.
    //
    //      float32 15361 (0x46700400)

    std::string stringValue = GetFormattedMessage("%10s ", elementTypeName);

    // Print numeric component.
    AppendFormattedNumericValue(elementType, floatValue, rawBitValue, valueFlags, /*inout*/ stringValue);

    stringValue.append(leftFlank);
    AppendFormattedNumericValue(elementType, rawBitValue, valueFlags, /*inout*/ stringValue);
    stringValue.append(rightFlank);

    return stringValue;
}

void PrintNumericType(
    ElementType elementType,
    const void* binaryData,
    std::string_view leftFlank,
    std::string_view rightFlank,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType originalElementType = ElementType::Undefined
)
{
    std::string s = GetFormattedNumericValue(elementType, binaryData, leftFlank, rightFlank, numericPrintingFlags);
    if (elementType == originalElementType)
    {
        printf(" -> %s\n", s.c_str());
    }
    else
    {
        printf("    %s\n", s.c_str());
    }
}

void PrintBytes(const void* binaryData, size_t binaryDataByteSize)
{
    printf("         bytes ");

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(binaryData);
    for (size_t i = 0; i < binaryDataByteSize; ++i)
    {
        printf("%02X ", bytes[i]);
    }

    printf("\n");
}

void PrintAllNumericTypesToBinary(
    double value,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType originalElementType = ElementType::Undefined
)
{
    constexpr std::string_view leftFlank = " -> ";
    constexpr std::string_view rightFlank = "";

    NumberUnion numberUnion = {};

    numberUnion.ui8  = static_cast<uint8_t >(value); PrintNumericType(ElementType::Uint8,  &numberUnion.ui8,  leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.ui16 = static_cast<uint16_t>(value); PrintNumericType(ElementType::Uint16, &numberUnion.ui16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.ui32 = static_cast<uint32_t>(value); PrintNumericType(ElementType::Uint32, &numberUnion.ui32, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.ui64 = static_cast<uint64_t>(value); PrintNumericType(ElementType::Uint64, &numberUnion.ui64, leftFlank, rightFlank, numericPrintingFlags, originalElementType);

    numberUnion.i8   = static_cast<int8_t >(value); PrintNumericType(ElementType::Int8,  &numberUnion.i8,  leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.i16  = static_cast<int16_t>(value); PrintNumericType(ElementType::Int16, &numberUnion.i16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.i32  = static_cast<int32_t>(value); PrintNumericType(ElementType::Int32, &numberUnion.i32, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.i64  = static_cast<int64_t>(value); PrintNumericType(ElementType::Int64, &numberUnion.i64, leftFlank, rightFlank, numericPrintingFlags, originalElementType);

    numberUnion.f16       = static_cast<float16_t>(float(value));  PrintNumericType(ElementType::Float16,  &numberUnion.f16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.f16m7e8s1 = static_cast<bfloat16_t>(float(value)); PrintNumericType(ElementType::Bfloat16, &numberUnion.f16m7e8s1, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.f32       = static_cast<float32_t>(value);         PrintNumericType(ElementType::Float32,  &numberUnion.f32, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.f64       = static_cast<float64_t>(value);         PrintNumericType(ElementType::Float64,  &numberUnion.f64, leftFlank, rightFlank, numericPrintingFlags, originalElementType);

    numberUnion.f12_12 = static_cast<Fixed12_12>(float(value)); PrintNumericType(ElementType::Fixed24f12i12, &numberUnion.f12_12, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.f16_16 = static_cast<Fixed16_16>(float(value)); PrintNumericType(ElementType::Fixed32f16i16, &numberUnion.f16_16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    numberUnion.f8_24  = static_cast<Fixed8_24>(float(value));  PrintNumericType(ElementType::Fixed32f24i8,  &numberUnion.f8_24,  leftFlank, rightFlank, numericPrintingFlags, originalElementType);
}

void PrintAllNumericTypesFromBinary(
    int64_t value,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType originalElementType = ElementType::Undefined
)
{
    constexpr std::string_view leftFlank = " <- ";
    constexpr std::string_view rightFlank = "";

    NumberUnion numberUnion = {};
    numberUnion.i64 = value;

    PrintNumericType(ElementType::Uint8,  &numberUnion.ui8,  leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Uint16, &numberUnion.ui16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Uint32, &numberUnion.ui32, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Uint64, &numberUnion.ui64, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    
    PrintNumericType(ElementType::Int8,   &numberUnion.i8,  leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Int16,  &numberUnion.i16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Int32,  &numberUnion.i32, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Int64,  &numberUnion.i64, leftFlank, rightFlank, numericPrintingFlags, originalElementType);

    PrintNumericType(ElementType::Float16,  &numberUnion.f16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Bfloat16, &numberUnion.f16m7e8s1,      leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Float32,  &numberUnion.f32, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Float64,  &numberUnion.f64, leftFlank, rightFlank, numericPrintingFlags, originalElementType);

    PrintNumericType(ElementType::Fixed24f12i12, &numberUnion.f12_12, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Fixed32f16i16, &numberUnion.f16_16, leftFlank, rightFlank, numericPrintingFlags, originalElementType);
    PrintNumericType(ElementType::Fixed32f24i8,  &numberUnion.f8_24,  leftFlank, rightFlank, numericPrintingFlags, originalElementType);
}

void PrintAllPrintingFormats(
    double valueFloat,
    int64_t valueInteger,
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default,
    ElementType elementType = ElementType::Undefined
)
{
    // Print the number in various representations.

    std::string stringValue;

    const char* elementTypeName = GetTypeNameFromElementType(elementType).data();

    printf("          type %s\n", elementTypeName);

    stringValue.clear();
    AppendFormattedNumericValue(elementType, valueFloat, valueInteger, NumericPrintingFlags::Default, /*inout*/ stringValue);
    printf("       decimal %s\n", stringValue.c_str());

    stringValue.clear();
    AppendFormattedNumericValue(elementType, valueFloat, valueInteger, NumericPrintingFlags::ShowFloatHex, /*inout*/ stringValue);
    printf("      floathex %s\n", stringValue.c_str());

    stringValue.clear();
    AppendFormattedNumericValue(elementType, valueInteger, NumericPrintingFlags::ShowRawHex, /*inout*/ stringValue);
    printf("           hex %s\n", stringValue.c_str());

    stringValue.clear();
    AppendFormattedNumericValue(elementType, valueInteger, NumericPrintingFlags::ShowRawOctal, /*inout*/ stringValue);
    printf("           oct %s\n", stringValue.c_str());

    stringValue.clear();
    AppendFormattedNumericValue(elementType, valueInteger, NumericPrintingFlags::ShowRawBinary, /*inout*/ stringValue);
    printf("           bin %s\n", stringValue.c_str());

    stringValue.clear();
    AppendFormattedNumericValue(elementType, valueInteger, NumericPrintingFlags::ShowRawBinaryFields, /*inout*/ stringValue);
    printf("    fields bin %s\n", stringValue.c_str());
}

void ParseNumber(
    _In_z_ const char* valueString,
    ElementType preferredElementType,
    bool parseAsRawData,
    _Out_ NumberUnionAndType& number
)
{
    number = {};
    number.elementType = preferredElementType;

    // Try to read the number as the preferred element type, or if undefined,
    // try to read the numbers as float64 and int64.

    const bool isUndefinedType = (preferredElementType == ElementType::Undefined);
    const bool isFractionalType = IsFractionalElementType(preferredElementType);
    const bool parseFloatingPoint = isUndefinedType | isFractionalType;

    char* valueStringEnd = nullptr;
    const double valueFloat = strtod(valueString, &valueStringEnd);
    const bool wasFloatValueParsed = (valueStringEnd > valueString);
    const bool wasDecimalPresent = wasFloatValueParsed && (std::find(valueString, const_cast<char const*>(valueStringEnd), '.') != valueStringEnd);
    const bool isValueFloatZero = (valueFloat == 0);

    // Read as signed or unsigned value.
    int64_t valueInt = 0;
    if (preferredElementType == ElementType::Undefined
    ||  (IsSignedElementType(preferredElementType) && !parseAsRawData)
    ||  valueString[0] == '-')
    {
        valueInt = strtoll(valueString, &valueStringEnd, 0);
    }
    else
    {
        valueInt = strtoull(valueString, &valueStringEnd, 0);
    }

    // Parse binary or octal numbers, e.g. 0b1101, 0o123. 'strtol' doesn't recognize these.
    if (valueInt == 0 && valueString[0] == '0')
    {
        uint32_t radix = 0;
        if (valueString[1] == 'b') radix = 2;
        if (valueString[1] == 'o') radix = 8;

        if (radix != 0)
        {
            valueInt = strtoull(valueString + 2, &valueStringEnd, radix);
        }
    }

    // If the type wasn't given, deduce from whether the number had a fraction.
    if (isUndefinedType)
    {
        // Treat as floating point if a decimal was present.
        if (wasDecimalPresent)
        {
            if (parseAsRawData)
            {
                number.numberUnion.i64 = valueInt;
            }
            else
            {
                number.numberUnion.f64 = valueFloat;
            }
            number.elementType = ElementType::Float64;
        }
        else if (valueInt <= INT32_MAX && valueInt >= -INT32_MAX)
        {
            number.numberUnion.i32 = static_cast<int32_t>(valueInt);
            number.elementType = ElementType::Int32;
        }
        else if (valueInt <= UINT32_MAX)
        {
            number.numberUnion.ui32 = static_cast<uint32_t>(valueInt);
            number.elementType = ElementType::Uint32;
        }
        else // Treat as 64-bit integer.
        {
            number.numberUnion.i64 = valueInt;
            number.elementType = ElementType::Int64;
        }
    }
    else if (isFractionalType)
    {
        if (parseAsRawData)
        {
            number.numberUnion.i64 = valueInt;
        }
        else if (isValueFloatZero)
        {
            WriteFromInt64(preferredElementType, valueInt, /*out*/ &number.numberUnion);
        }
        else
        {
            WriteFromDouble(preferredElementType, valueFloat, /*out*/ &number.numberUnion);
        }
    }
    else // integer
    {
        WriteFromInt64(preferredElementType, valueInt, /*out*/ &number.numberUnion);
    }
}

void PrintAllNumbers(Span<const NumberUnionAndType> numbers)
{
    constexpr std::string_view leftFlank = " (";
    constexpr std::string_view rightFlank = ")";

    for (NumberUnionAndType const& number : numbers)
    {
        if (number.elementType != ElementType::Undefined)
        {
            std::string s = GetFormattedNumericValue(number.elementType, &number.numberUnion, leftFlank, rightFlank, number.printingFlags);
            printf("    %s\n", s.c_str());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

struct INumericOperationPerformer
{
    virtual void Add(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Subtract(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Multiply(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Divide(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
    virtual void Dot(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) = 0;
};

template <typename T>
T& CastNumberType(NumberUnionAndType const& input, _Inout_ NumberUnionAndType& output)
{
    ReadWriteElementType(input.elementType, output.elementType, input.numberUnion.buffer, output.numberUnion.buffer);
    return CastReferenceAs<T>(output.numberUnion.buffer);
}

template <typename T>
class NumericOperationPerformer : public INumericOperationPerformer
{
    void Add(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0);
        NumberUnionAndType intermediate = finalResult; // Intermediate needed for casting.
        
        for (NumberUnionAndType const& n : numbers)
        {
            result += CastNumberType<T>(n, /*inout*/ intermediate);
        }
        CastReferenceAs<T>(finalResult) = result;
    }

    void Subtract(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0);
        NumberUnionAndType intermediate = finalResult;

        if (!numbers.empty())
        {
            result = CastNumberType<T>(numbers.front(), /*inout*/ intermediate);

            numbers.PopFront();
            for (NumberUnionAndType const& n : numbers)
            {
                result -= CastNumberType<T>(n, /*inout*/ intermediate);
            }
        }
        CastReferenceAs<T>(finalResult) = result;
    };

    void Multiply(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(1);
        NumberUnionAndType intermediate = finalResult;

        for (auto& n : numbers)
        {
            result *= CastNumberType<T>(n, /*inout*/ intermediate);
        }
        CastReferenceAs<T>(finalResult) = result;
    };

    void Divide(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0);
        NumberUnionAndType intermediate = finalResult;

        if (!numbers.empty())
        {
            result = CastNumberType<T>(numbers.front(), intermediate);

            numbers.PopFront();
            for (NumberUnionAndType const& n : numbers)
            {
                result /= CastNumberType<T>(n, /*inout*/ intermediate);
            }
        }
        CastReferenceAs<T>(finalResult) = result;
    };

    void Dot(Span<const NumberUnionAndType> numbers, _Out_ NumberUnionAndType& finalResult) override
    {
        T result = T(0); 
        size_t i = 0;
        size_t numberCount = numbers.size();
        size_t evenNumberCount = numbers.size() & ~size_t(1);
        NumberUnionAndType intermediate1 = finalResult;
        NumberUnionAndType intermediate2 = finalResult;

        for (i = 0; i < evenNumberCount; i += 2)
        {
            T product = CastNumberType<T>(numbers[i],     /*out*/ intermediate1)
                      * CastNumberType<T>(numbers[i + 1], /*out*/ intermediate2);
            result += product;
        }
        if (i < numberCount)
        {
            result += CastNumberType<T>(numbers[i], /*out*/ intermediate1);
        }
        CastReferenceAs<T>(finalResult) = result;

    #if 0 // Experimental rounding.
        constexpr uint64_t mantissaMaskLowBits = (0x1ui64 << 42) - 1;
        constexpr uint64_t mantissaMaskHighBits = ~mantissaMaskLowBits;
        constexpr uint64_t mantissaEvenBit = 0x1ui64 << 42;
        constexpr uint64_t mantissaMaskRoundingBit = 1ui64 << (42 - 1);

        // Round halves to evens
        NumberUnion result;
        result.f64 = double(0);
        size_t i = 0;
        size_t numberCount = numbers.size();
        size_t evenNumberCount = numbers.size() & ~size_t(1);

        for (i = 0; i < evenNumberCount; i += 2)
        {
            NumberUnion product;
            product.f64 = double(CastAs<T>(numbers[i])) * double(CastAs<T>(numbers[i + 1]));

            if ((product.ui64 & mantissaMaskLowBits) == mantissaMaskRoundingBit)
            {
                product.ui64 += (product.ui64 & mantissaEvenBit) ? -int64_t(mantissaMaskRoundingBit) : mantissaMaskRoundingBit;
            }
            result.f64 += product.f64;
            if ((result.ui64 & mantissaMaskLowBits) == mantissaMaskRoundingBit)
            {
                result.ui64 += (result.ui64 & mantissaEvenBit) ? -int64_t(mantissaMaskRoundingBit) : mantissaMaskRoundingBit;
            }
        }
        if (i < numberCount)
        {
            result.f64 += CastAs<T>(numbers[i]);
            if ((result.ui64 & mantissaMaskLowBits) == mantissaMaskRoundingBit)
            {
                result.ui64 += (result.ui64 & mantissaEvenBit) ? -int64_t(mantissaMaskRoundingBit) : mantissaMaskRoundingBit;
            }
        }
        CastAs<T>(finalResult) = T(float(result.f64));

        // Round halves towards infinity
        NumberUnion result;
        result.f64 = double(0);
        size_t i = 0;
        size_t numberCount = numbers.size();
        size_t evenNumberCount = numbers.size() & ~size_t(1);

        for (i = 0; i < evenNumberCount; i += 2)
        {
            NumberUnion product;
            product.f64 = double(CastAs<T>(numbers[i])) * double(CastAs<T>(numbers[i + 1]));
            product.ui64 += (0x1ui64 << 41);
            product.ui64 &= ~((0x1ui64 << 42) - 1);
            result.f64 += product.f64;
            result.ui64 += (0x1ui64 << 41);
            result.ui64 &= ~((0x1ui64 << 42) - 1);
        }
        if (i < numberCount)
        {
            result.f64 += CastAs<T>(numbers[i]);
            result.ui64 += (0x1ui64 << 41);
            result.ui64 &= ~((0x1ui64 << 42) - 1);
        }
        CastAs<T>(finalResult) = T(float(result.f64));
    #endif
    };
};

// Declare singletons since they are stateless anyway.
NumericOperationPerformer<float> g_numericOperationPerformerFloat32;
NumericOperationPerformer<double> g_numericOperationPerformerFloat64;
NumericOperationPerformer<float16_t> g_numericOperationPerformerFloat16;
NumericOperationPerformer<float16m7e8s1_t> g_numericOperationPerformerFloat16m7e8s1;
NumericOperationPerformer<uint8_t> g_numericOperationPerformerUint8;
NumericOperationPerformer<uint16_t> g_numericOperationPerformerUint16;
NumericOperationPerformer<uint32_t> g_numericOperationPerformerUint32;
NumericOperationPerformer<uint64_t> g_numericOperationPerformerUint64;
NumericOperationPerformer<int8_t> g_numericOperationPerformerInt8;
NumericOperationPerformer<int16_t> g_numericOperationPerformerInt16;
NumericOperationPerformer<int32_t> g_numericOperationPerformerInt32;
NumericOperationPerformer<int64_t> g_numericOperationPerformerInt64;
NumericOperationPerformer<Fixed12_12> g_numericOperationPerformerFixed24f12i12;
NumericOperationPerformer<Fixed16_16> g_numericOperationPerformerFixed32f16i16;
NumericOperationPerformer<Fixed8_24> g_numericOperationPerformerFixed32f24i8;

ElementType GetPromotedOutputElementType(Span<const NumberUnionAndType> numbers)
{
    // Determine the output element type based on the priority of each pair of types.
    // If all data types are identical or there is only one, the result is simply the first input type.

    ElementType outputElementType = ElementType::Undefined;
    if (!numbers.empty())
    {
        outputElementType = numbers.front().elementType;
        numbers.PopFront();

        for (auto& number : numbers)
        {
            ElementType inputElementType = number.elementType;
            outputElementType = g_elementTypePriorityTable[size_t(inputElementType)] > g_elementTypePriorityTable[size_t(outputElementType)]
                ? inputElementType
                : outputElementType;
        }
    }

    return outputElementType;
}

void PerformNumericOperation(
    NumericOperationType numericOperationType,
    Span<const NumberUnionAndType> numbers,
    _Inout_ NumberUnionAndType& result
)
{
    if (numbers.empty())
    {
        result.numberUnion = {};
        return;
    }

    INumericOperationPerformer* performer = &g_numericOperationPerformerFloat32;

    // Determine the output element type based on the inputs.
    if (result.elementType == ElementType::Undefined)
    {
        result.elementType = GetPromotedOutputElementType(numbers);
    }

    // Choose the respective operation performer based on data type.
    switch (result.elementType)
    {
    case ElementType::Undefined:        return;
    case ElementType::Float32:          performer = &g_numericOperationPerformerFloat32; break;
    case ElementType::Uint8:            performer = &g_numericOperationPerformerUint8; break;
    case ElementType::Int8:             performer = &g_numericOperationPerformerInt8; break;
    case ElementType::Uint16:           performer = &g_numericOperationPerformerUint16; break;
    case ElementType::Int16:            performer = &g_numericOperationPerformerInt16; break;
    case ElementType::Int32:            performer = &g_numericOperationPerformerInt32; break;
    case ElementType::Int64:            performer = &g_numericOperationPerformerInt64; break;
    case ElementType::StringChar8:      return;
    case ElementType::Bool8:            return;
    case ElementType::Float16:          performer = &g_numericOperationPerformerFloat16; break;
    case ElementType::Float64:          performer = &g_numericOperationPerformerFloat64; break;
    case ElementType::Uint32:           performer = &g_numericOperationPerformerUint32; break;
    case ElementType::Uint64:           performer = &g_numericOperationPerformerUint64; break;
    case ElementType::Complex64:        return;
    case ElementType::Complex128:       return;
    case ElementType::Float16m7e8s1:    performer = &g_numericOperationPerformerFloat16m7e8s1; break;
    case ElementType::Fixed24f12i12:    performer = &g_numericOperationPerformerFixed24f12i12; break;
    case ElementType::Fixed32f16i16:    performer = &g_numericOperationPerformerFixed32f16i16; break;
    case ElementType::Fixed32f24i8:     performer = &g_numericOperationPerformerFixed32f24i8; break;
    }

    switch (numericOperationType)
    {
    case NumericOperationType::Add:         performer->Add(numbers, /*out*/ result); break;
    case NumericOperationType::Subtract:    performer->Subtract(numbers, /*out*/ result); break;
    case NumericOperationType::Multiply:    performer->Multiply(numbers, /*out*/ result); break;
    case NumericOperationType::Divide:      performer->Divide(numbers, /*out*/ result); break;
    case NumericOperationType::Dot:         performer->Dot(numbers, /*out*/ result); break;
    }
}

////////////////////////////////////////////////////////////////////////////////

void PrintUsage()
{
    printf(
        "Usage:\n"
        "   binums 12.75  // floating point value in various formats\n"
        "   binums 0b1101  // read binary integer\n"
        "   binums float32 raw 0x40490FDB  // read raw floating point bits\n"
        "   binums float16 raw 0x5140  // read raw floating point bits\n"
        "   binums fields hex 7 12.75 -13 bin 7 12.75 -13  // see fields of numbers\n"
        "   binums int8 fields 13 -13  // see fields of numbers\n"
        "   binums uint32 add 1.5 3.25  // perform operation\n"
        "   binums float32 add float16 2 3  // read float16, compute in float32\n"
        "   binums uint32 mul 3 2 add 3 2 subtract 3 2 dot 1 2 3 4\n"
        "   binums 0x1.5p5  // floating point hexadecimal\n"
        "   binums fixed12_12 sub 3.5 2  // fixed point arithmetic\n"
        "\n"
        "Options:\n"
        "   bin hex dec oct - display raw bits as binary/hex/decimal/octal\n"
        "   floathex floatdec - display floating values as hex or decimal (default)\n"
        "   raw num - read input as raw bit data or as number (default)\n"
        "   fields nofields - show numeric component bitfields\n"
        "   add subtract multiply divide dot nop - apply operation to following numbers\n"
        "   float16 bfloat16 float32 float64 - set floating point data type\n"
        "   uint8 uint16 uint32 uint64 int8 int16 int32 int64 - set integer data type\n"
        "   fixed12_12 fixed16_16 fixed8_24 - set fixed precision data type\n"
        "\n"
        "Dwayne Robinson, 2019-02-14..2019-12-19, No Copyright\n"
        "https://github.com/fdwr/BiNums\n"
    );
}

int ParseCommandLineParameters(
    int argc,
    char* argv[],
    _Out_ std::vector<NumericOperationAndRange>& operations,
    _Out_ std::vector<NumberUnionAndType>& numbers
)
{
    const char* valueString = "0";
    NumberUnionAndType numberUnionAndType;
    bool parseAsRawData = false;
    ElementType preferredElementType = ElementType::Undefined;
    NumericPrintingFlags numericPrintingFlags = NumericPrintingFlags::Default;

    operations.clear();
    numbers.clear();

    if (argc >= 2)
    {
        for (size_t i = 1; i < size_t(argc); ++i)
        {
            std::string_view param(argv[i]);

            NumericOperationAndRange numericOperationAndRange = {};

            // Check if ordinary number or operator.
            if (isdigit(param.front()) || (param.front() == '-' && isdigit(param[1])))
            {
                auto value = param.begin();
                while (value != param.end())
                {
                    ParseNumber(&*value, preferredElementType, parseAsRawData, /*out*/ numberUnionAndType);
                    numberUnionAndType.printingFlags = numericPrintingFlags;
                    numbers.push_back(numberUnionAndType);
                    value = std::find(value, param.end(), ',');
                    if (value == param.end())
                    {
                        break;
                    }
                    else
                    {
                        ++value; // Skip the comma.
                    }
                }
            }
            else if (param == "nop"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::None;
            }
            else if (param == "add"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Add;
            }
            else if (param == "sub"sv || param == "subtract"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Subtract;
            }
            else if (param == "mul"sv || param == "multiply"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Multiply;
            }
            else if (param == "div"sv || param == "divide"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Divide;
            }
            else if (param == "dot"sv || param == "dotproduct"sv)
            {
                numericOperationAndRange.numericOperationType = NumericOperationType::Dot;
            }
            else if (param == "raw"sv)
            {
                parseAsRawData = true;
            }
            else if (param == "num"sv)
            {
                parseAsRawData = false;
            }
            else if (param == "undefined"sv)
            {
                preferredElementType = ElementType::Undefined;
            }
            else if (param == "i8"sv || param == "int8"sv)
            {
                preferredElementType = ElementType::Int8;
            }
            else if (param == "ui8"sv || param == "uint8"sv)
            {
                preferredElementType = ElementType::Uint8;
            }
            else if (param == "i16"sv || param == "int16"sv)
            {
                preferredElementType = ElementType::Int16;
            }
            else if (param == "ui16"sv || param == "uint16"sv)
            {
                preferredElementType = ElementType::Uint16;
            }
            else if (param == "i32"sv || param == "int32"sv || param == "int"sv)
            {
                preferredElementType = ElementType::Int32;
            }
            else if (param == "ui32"sv || param == "uint32"sv || param == "uint"sv)
            {
                preferredElementType = ElementType::Uint32;
            }
            else if (param == "i64"sv || param == "int64"sv)
            {
                preferredElementType = ElementType::Int64;
            }
            else if (param == "ui64"sv || param == "uint64"sv)
            {
                preferredElementType = ElementType::Uint64;
            }
            else if (param == "f16"sv || param == "float16"sv)
            {
                preferredElementType = ElementType::Float16;
            }
            else if (param == "f16m7e8s1"sv || param == "bfloat16"sv)
            {
                preferredElementType = ElementType::Float16m7e8s1;
            }
            else if (param == "f32"sv || param == "float32"sv || param == "float"sv)
            {
                preferredElementType = ElementType::Float32;
            }
            else if (param == "f64"sv || param == "float64"sv || param == "double"sv)
            {
                preferredElementType = ElementType::Float64;
            }
            else if (param == "fixed12_12"sv)
            {
                preferredElementType = ElementType::Fixed24f12i12;
            }
            else if (param == "fixed16_16"sv)
            {
                preferredElementType = ElementType::Fixed32f16i16;
            }
            else if (param == "fixed8_24"sv)
            {
                preferredElementType = ElementType::Fixed32f24i8;
            }
            else if (param == "bin"sv || param == "binary"sv || param == "showrawbinary"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawBinary);
            }
            else if (param == "hex"sv || param == "hexademical"sv || param == "showrawhexadecimal"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawHex);
            }
            else if (param == "dec"sv || param == "decimal"sv || param == "showrawdecimal"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawDecimal);
            }
            else if (param == "oct"sv || param == "octal"sv || param == "showrawoctal"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowDataMask, NumericPrintingFlags::ShowRawOctal);
            }
            else if (param == "floathex"sv || param == "showfloathexadecimal"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowFloatMask, NumericPrintingFlags::ShowFloatHex);
            }
            else if (param == "floatdec"sv || param == "showfloatdecimal"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowFloatMask, NumericPrintingFlags::ShowFloatDecimal);
            }
            else if (param == "fields"sv || param == "showrawfields"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowRawFieldsMask, NumericPrintingFlags::ShowRawFields);
            }
            else if (param == "nofields"sv || param == "hiderawfields"sv)
            {
                numericPrintingFlags = SetFlags(numericPrintingFlags, NumericPrintingFlags::ShowRawFieldsMask, NumericPrintingFlags::HideRawFields);
            }
            else
            {
                printf("Unknown parameter: \"%.*s\"\n", int(param.size()), param.data());
                PrintUsage();
                return EXIT_FAILURE;
            }

            // Append any new numeric operations.
            if (numericOperationAndRange.numericOperationType != NumericOperationType::None)
            {
                const uint32_t numberCount = static_cast<uint32_t>(numbers.size());
                if (!operations.empty())
                {
                    operations.back().range.end = numberCount;
                }
                numericOperationAndRange.range.begin = numberCount;
                numericOperationAndRange.range.end = numberCount;
                numericOperationAndRange.outputElementType = preferredElementType;
                operations.push_back(numericOperationAndRange);
            }
        }

        const uint32_t numberCount = static_cast<uint32_t>(numbers.size());
        if (!operations.empty())
        {
            operations.back().range.end = numberCount;
        }
    }
    else
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    std::vector<NumericOperationAndRange> operations;
    std::vector<NumberUnionAndType> numbers;

    int exitCode = ParseCommandLineParameters(argc, argv, /*out*/ operations, /*out*/ numbers);
    if (exitCode != EXIT_SUCCESS)
    {
        return exitCode;
    }

    if (!operations.empty())
    {
        // Process every operation in order.
        for (auto& operation : operations)
        {
            _Null_terminated_ const char* numericOperationName = GetNumericOperationNameFromNumericOperationType(operation.numericOperationType).data();

            // Print the operands.
            printf("Operands to %s:\n", numericOperationName);
            Span<const NumberUnionAndType> span(numbers.data() + operation.range.begin, numbers.data() + operation.range.end);
            PrintAllNumbers(span);

            // Process the values.
            NumberUnionAndType operationResult = {};
            operationResult.elementType = operation.outputElementType;
            operationResult.printingFlags = span.empty() ? NumericPrintingFlags::Default : span.front().printingFlags;
            PerformNumericOperation(operation.numericOperationType, span, /*out*/ operationResult);

            // Print the result.
            printf("Result from %s:\n", numericOperationName);
            Span<const NumberUnionAndType> singleElementSpan(&operationResult, 1);
            PrintAllNumbers(singleElementSpan);
            printf("\n");
        }
    }
    else if (numbers.size() == 1)
    {
        // If exactly one number is given, show it in possible formats.
        auto& numberUnion = numbers.front();
        double valueFloat = ReadToDouble(numberUnion.elementType, &numberUnion.numberUnion);
        int64_t valueInteger = ReadRawBitValue(numberUnion.elementType, &numberUnion.numberUnion);

        printf("Representations:\n");
        PrintAllPrintingFormats(valueFloat, valueInteger, numberUnion.printingFlags, numberUnion.elementType);

        printf("\n" "To binary:\n");
        PrintAllNumericTypesToBinary(valueFloat, numberUnion.printingFlags, numberUnion.elementType);

        printf("\n" "From binary:\n");
        PrintAllNumericTypesFromBinary(valueInteger, numberUnion.printingFlags, numberUnion.elementType);
    }
    else if (!numbers.empty())
    {
        // If multiple numbers are given, print them all.
        PrintAllNumbers(Span<const NumberUnionAndType>(numbers.data(), numbers.size()));
    }

    return EXIT_SUCCESS;
}
