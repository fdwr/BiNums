// BiNums, see binary numbers

#include "precomp.h"
#if _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

extern int MainImplementation(std::string_view commandLine, std::string& stringOutput);

////////////////////////////////////////////////////////////////////////////////
// Generic functions/classes.

bool StringsMatch(char const* a, char const* b)
{
    return strcmp(a, b) == 0;
}

// TODO: Add VTT codes for Linux.
struct SetAndSaveConsoleAttribute
{
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo_;
    HANDLE hConsole_;
    #endif

    SetAndSaveConsoleAttribute()
    {
        #ifdef _WIN32
        hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hConsole_, &consoleInfo_);
        #endif
    }

    SetAndSaveConsoleAttribute(WORD newAttributes) : SetAndSaveConsoleAttribute()
    {
        UpdateAttributes(newAttributes);
    }

    SetAndSaveConsoleAttribute(WORD setAttributes, WORD clearAttributes) : SetAndSaveConsoleAttribute()
    {
        UpdateAttributes(setAttributes, clearAttributes);
    }

    void UpdateForegroundColor(WORD foregroundColor)
    {
        UpdateAttributes(foregroundColor & 0x000F, 0x000F);
    }

    void UpdateAttributes(WORD newAttributes)
    {
        #ifdef _WIN32
        SetConsoleTextAttribute(hConsole_, newAttributes);
        #endif
    }

    void UpdateAttributes(WORD setAttributes, WORD clearAttributes)
    {
        #ifdef _WIN32
        SetConsoleTextAttribute(hConsole_, (consoleInfo_.wAttributes & ~clearAttributes) | setAttributes);
        #endif
    }

    void Reset()
    {
        #ifdef _WIN32
        SetConsoleTextAttribute(hConsole_, consoleInfo_.wAttributes);
        #endif
    }

    ~SetAndSaveConsoleAttribute()
    {
        #ifdef _WIN32
        SetConsoleTextAttribute(hConsole_, consoleInfo_.wAttributes);
        #endif
    }
};


struct BiNumsTest
{
    bool shouldRegenerateExpectedBaseline = false;
};

BiNumsTest g_test;

int ParseCommandLineParameters(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        char const* argument = argv[i];
        if (StringsMatch(argument, "/?") || StringsMatch(argument, "-h") || StringsMatch(argument, "-help"))
        {
            printf(
                "Run without any arguments to execute the tests.\n"
                "Failed test results go into 'TestResults\\'.\n"
                "\n"
                "regenerate : regenerate the test cases (use git diff afterward to verify differences)\n"
            );
            return EXIT_FAILURE;
        }
        else if (StringsMatch(argument, "regenerate"))
        {
            g_test.shouldRegenerateExpectedBaseline = true;
        }
        else
        {
            printf(
                "Unrecognized parameter: %s\n"
                "Type -help for help.",
                argument
            );
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void PrintIndentedText(
    std::string_view text,
    std::string_view indentPrefix
    )
{
    if (indentPrefix.empty())
    {
        fwrite(text.data(), 1, text.size(), stdout);
    }

    std::string indentedText;
    for (auto p = text.begin(), end = text.end(); p != end; )
    {
        indentedText.append(indentPrefix);

        auto endOfLine = std::find(p, end, '\n');
        if (endOfLine != end)
        {
            ++endOfLine;
        }
        indentedText.append(&*p, endOfLine - p);
        p = endOfLine;
    }
    if (indentedText.empty() || indentedText.back() != '\n')
    {
        indentedText.push_back('\n');
    }

    fwrite(indentedText.data(), 1, indentedText.size(), stdout);
}

bool CompareExpectedVsActual(
    std::string_view testTitle,
    std::string_view actual,
    std::string_view expected
    )
{
    bool stringsMatch = (expected == actual);

    // Display colored OK/FAILED.
    SetAndSaveConsoleAttribute consoleAttributes;
    consoleAttributes.UpdateForegroundColor(stringsMatch ? FOREGROUND_GREEN : FOREGROUND_RED);
    printf(stringsMatch ? "OK    " : "FAILED");
    consoleAttributes.Reset();

    printf("  %.*s\n", static_cast<int>(testTitle.size()), testTitle.data());
    if (!stringsMatch)
    {
        printf("        ");
        consoleAttributes.UpdateAttributes(COMMON_LVB_UNDERSCORE);
        printf("Expected output:\n");
        consoleAttributes.Reset();
        PrintIndentedText(expected, "        |");

        printf("        ");
        consoleAttributes.UpdateAttributes(COMMON_LVB_UNDERSCORE);
        printf("Actual output:\n");
        consoleAttributes.Reset();
        PrintIndentedText(actual, "        |");
    }

    return stringsMatch;
}


bool VerifyFloatingTypes()
{
    constexpr double testNumbersFloat8f3e4s1[] = {
        0.0f,
        1.0f,
       -1.0f,
        0.5f,
       -0.5f,
        224.0f,
       -224.0f,
        std::numeric_limits<float>::quiet_NaN(),
       -std::numeric_limits<float>::quiet_NaN(),
        // Exclude infinity because this type doesn't have it.
        // So it would not round-trip successfully.
        // std::numeric_limits<float>::infinity(),
        //-std::numeric_limits<float>::infinity()
    };
    constexpr double testNumbersFloat8f2e5s1[] = {
        0.0f,
        1.0f,
       -1.0f,
        0.5f,
       -0.5f,
        224.0f, // Maximum value
       -224.0f, // Maximum value
        std::numeric_limits<float>::quiet_NaN(),
       -std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
       -std::numeric_limits<float>::infinity()
    };
    constexpr double testNumbersFloat16[] = {
        0.0f,
        1.0f,
       -1.0f,
        0.5f,
       -0.5f,
        65504.0f, // Maximum precise value
       -65504.0f, // Maximum precise value
        std::numeric_limits<float>::quiet_NaN(),
       -std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
       -std::numeric_limits<float>::infinity()
    };
    constexpr double testNumbersBFloat16[] = {
        0.0f,
        1.0f,
       -1.0f,
        0.5f,
       -0.5f,
        65280.0f, // Maximum value
       -65280.0f, // Maximum value
        std::numeric_limits<float>::quiet_NaN(),
       -std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
       -std::numeric_limits<float>::infinity()
    };
    constexpr double testNumbersFloat32[] = {
        0.0,
        1.0,
       -1.0,
        0.5,
       -0.5,
        65504.0,
       -65504.0,
        16777216.0, // Maximum precise value
       -16777216.0, // Maximum precise value
        std::numeric_limits<float>::min(),
       -std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max(),
       -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::quiet_NaN(),
       -std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
       -std::numeric_limits<float>::infinity()
    };
    constexpr auto& testNumbersFloat64 = testNumbersFloat32;

    bool success = true;

    SetAndSaveConsoleAttribute consoleAttributes;

    auto PrintResult = [&](char const* title, double a, double b)
    {
        bool valuesMatch = (a == b) || (std::isnan(a) && std::isnan(b));
        success &= valuesMatch;
        consoleAttributes.UpdateForegroundColor(valuesMatch ? FOREGROUND_GREEN : FOREGROUND_RED);
        printf(
            valuesMatch ? "OK     - %s - %g == %g\n"
                        : "FAILED - %s - %g != %g\n",
            title,
            a,
            b
        );
        consoleAttributes.Reset();
    };

    // TODO: Fix FloatNumber to handle atypical NaN case with a single NaN representation.
    for (double originalValue : testNumbersFloat8f3e4s1)
    {
        FloatNumber<uint8_t, 3, 4, true, true, false, true> convertedValue = float(originalValue);
        float reconvertedValue = convertedValue;
        PrintResult("float32 to float8m3e4s1", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersFloat8f2e5s1)
    {
        FloatNumber<uint8_t, 2, 5, true, true, true, true> convertedValue = float(originalValue);
        float reconvertedValue = convertedValue;
        PrintResult("float32 to float8m2e5s1", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersFloat16)
    {
        FloatNumber<uint16_t, 10, 5, true, true, true, true> convertedValue = float(originalValue);
        float reconvertedValue = convertedValue;
        PrintResult("float32 to float16", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersFloat16)
    {
        FloatNumber<uint16_t, 10, 5, true, true, true, true> convertedValue(originalValue);
        double reconvertedValue = convertedValue;
        PrintResult("float64 to float16", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersBFloat16)
    {
        FloatNumber<uint16_t, 7, 8, true, true, true, true> convertedValue = float(originalValue);
        float reconvertedValue = convertedValue;
        PrintResult("float32 to bfloat16", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersBFloat16)
    {
        FloatNumber<uint16_t, 10, 5, true, true, true, true> convertedValue = float(originalValue);
        FloatNumber<uint16_t, 7, 8, true, true, true, true> convertedValue2 = float(convertedValue);
        FloatNumber<uint16_t, 10, 5, true, true, true, true> convertedValue3 = float(convertedValue2);
        float reconvertedValue = convertedValue3;
        PrintResult("float16 to bfloat16", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersFloat32)
    {
        FloatNumber<uint32_t, 23, 8, true, true, true, true> convertedValue = float(originalValue);
        float reconvertedValue = convertedValue;
        PrintResult("float32 to float32", originalValue, reconvertedValue);
    }

    for (double originalValue : testNumbersFloat32)
    {
        FloatNumber<uint32_t, 23, 8, true, true, true, true> convertedValue(originalValue);
        double reconvertedValue = convertedValue;
        PrintResult("float64 to float32", originalValue, reconvertedValue);
    }

    // The custom float64 should behave identical to double (or std::float64_t).
    for (double originalValue : testNumbersFloat64)
    {
        FloatNumber<uint64_t, 52, 11, true, true, true, true> convertedValue(originalValue);
        double reconvertedValue = convertedValue;
        PrintResult("float64 to float64", originalValue, reconvertedValue);
    }

    return success;
}


int main(int argc, char* argv[])
{
    printf("*** This test suite is just a skeleton for now. ***\n\n");

    auto exitCode = ParseCommandLineParameters(argc, argv);
    if (exitCode != EXIT_SUCCESS)
    {
        return exitCode;
    }

    // Placeholder for real tests, which should be data driven in a test file,
    // preferably JSON or XML (XML might simpler given all the new lines).
    std::string stringOutput;
    exitCode = MainImplementation("uint8 42 int8 -42 uint16 42 int16 -42 uint32 42 int32 -42 uint64 42 int64 -42 fixed12_12 -42.25 fixed16_16 -42.25 fixed8_24 -42.25 float16 -42.25 float32 -42.25 float64 -42.25", stringOutput);

    uint32_t errorCount = 0;
    auto CheckFailure = [&](bool success)
    {
        errorCount += success ? 0 : 1;
    };

    char const* expectedOutput =
        "         uint8 42 (0x2A)\n"
        "          int8 -42 (0xD6)\n"
        "        uint16 42 (0x002A)\n"
        "         int16 -42 (0xFFD6)\n"
        "        uint32 42 (0x0000002A)\n"
        "         int32 -42 (0xFFFFFFD6)\n"
        "        uint64 42 (0x000000000000002A)\n"
        "         int64 -42 (0xFFFFFFFFFFFFFFD6)\n"
        "    fixed12_12 -42.25 (0xFD5C00)\n"
        "    fixed16_16 -42.25 (0xFFD5C000)\n"
        "     fixed8_24 -42.25 (0xD5C00000)\n"
        "       float16 -42.25 (0xD148)\n"
        "       float32 -42.25 (0xC2290000)\n"
        "       float64 -42.25 (0xC045200000000000)\n"
        ;
    CheckFailure(CompareExpectedVsActual("All data types", stringOutput, expectedOutput));
    CheckFailure(CompareExpectedVsActual("Expected failure case to verify output comparison", stringOutput, "Gibberish just to verify failure"));

    CheckFailure(VerifyFloatingTypes());

    return EXIT_SUCCESS;
}
