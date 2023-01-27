// BiNums main, separated out from BiNums just so tests and main executable don't have both a main function.

#include "precomp.h"

extern int MainImplementation(std::string_view commandLine, std::string& stringOutput);
extern std::string ConcatenateCommandLineParameters(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    // Standard C/C++ tries to be helpful by chopping up the arguments for us,
    // but the general parsing function just accepts a string directly, which
    // could come from other sources (like test cases). So restore the original
    // back to the original string.
    std::string commandLine = ConcatenateCommandLineParameters(argc, argv);

    std::string stringOutput;
    int exitCode = MainImplementation(commandLine, /*out*/ stringOutput);
    std::fputs(stringOutput.c_str(), stdout);

    return exitCode;
}
