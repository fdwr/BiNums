// BiNums, see binary numbers

#include "precomp.h"
#if _WIN32
#include <Windows.h>
#endif

extern int MainImplementation(std::string_view commandLine, std::string& stringOutput);

////////////////////////////////////////////////////////////////////////////////
// Generic functions/classes.

bool StringsMatch(char const* a, char const* b)
{
    return strcmp(a, b) == 0;
}

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
    #if _WIN32
    // TODO: Add VTT codes for Linux.
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo = {};
    GetConsoleScreenBufferInfo(outputHandle, /*out*/ &consoleScreenBufferInfo);
    SetConsoleTextAttribute(outputHandle, stringsMatch ? FOREGROUND_GREEN : FOREGROUND_RED);
    #endif

    printf(stringsMatch ? "OK    " : "FAILED");

    // Restore original color.
    // TODO: Use RAII to clean up any potential exceptions.
    #if _WIN32
    SetConsoleTextAttribute(outputHandle, consoleScreenBufferInfo.wAttributes);
    #endif

    printf("  %.*s\n", static_cast<int>(testTitle.size()), testTitle.data());
    if (!stringsMatch)
    {
        printf("        ");
        #if _WIN32
        SetConsoleTextAttribute(outputHandle, COMMON_LVB_UNDERSCORE|consoleScreenBufferInfo.wAttributes);
        #endif
        printf("Expected output:\n");
        #if _WIN32
        SetConsoleTextAttribute(outputHandle, consoleScreenBufferInfo.wAttributes);
        #endif
        PrintIndentedText(expected, "        |");

        printf("        ");
        #if _WIN32
        SetConsoleTextAttribute(outputHandle, COMMON_LVB_UNDERSCORE|consoleScreenBufferInfo.wAttributes);
        #endif
        printf("Actual output:\n");
        #if _WIN32
        SetConsoleTextAttribute(outputHandle, consoleScreenBufferInfo.wAttributes);
        #endif
        PrintIndentedText(actual, "        |");
    }

    return stringsMatch;
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
    CompareExpectedVsActual("All data types", stringOutput, expectedOutput);
    CompareExpectedVsActual("Expected failure case to verify output comparison", stringOutput, "Gibberish just to verify failure");

    return EXIT_SUCCESS;
}
