#include <fstream>

#include "Extractor.h"

#include <clang/Tooling/Tooling.h>
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: headless [file.hpp]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }
    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::vector<std::string> args = {
        "-std=c++17",
        "-fsyntax-only",  // ✅ Only check syntax, don't expand macros/includes
        "-ffreestanding",  // ✅ Tell Clang not to assume standard library presence
        "-Wno-everything", // ✅ Suppress all warnings
        "-nostdinc",       // ✅ Completely disable standard includes
        "-nostdinc++",     // ✅ Disable C++ standard includes
        "-E"               // ✅ Keep preprocessor directives untouched (raw mode)
    };

    bool success = clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<ExtractAction>(), code, args, filename
    );

    if (!success) {
        std::cerr << "Error processing file " << filename << std::endl;
        return 1;
    }

    std::cout << "Extraction complete!" << std::endl;
    return 0;
}
