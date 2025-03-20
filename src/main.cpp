#include <fstream>

#include "Generator.h"

#include <clang/Tooling/Tooling.h>
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: headless <file.hpp>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }
    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Clang arguments (only syntax checking, no includes)
    std::vector<std::string> args = {
        "-std=c++17",
        "-fsyntax-only",  // Only check syntax
        "-fms-extensions",  // ✅ Allow unknown type parsing without errors
       "-fno-delayed-template-parsing", // ✅ Ensures Clang doesn’t try to instantiate templates
       "-fno-elide-type",  // ✅ Keeps all type names exactly as written
        "-Wno-everything",
         "-ffreestanding",
         "-Wno-pragma-once-outside-header",  // ✅ Ignore #pragma once warnings
         "-Wno-invalid-source-encoding",  // ✅ Ignore invalid encoding warnings
         "-Wno-unknown-warning-option" // ✅ Ignore unknown warning options
        "-nostdinc",       // ✅ Disable standard system includes
        "-nostdinc++"      // ✅ Disable standard C++ includes
    };

    bool success = clang::tooling::runToolOnCodeWithArgs(
        std::make_unique<Generator>(), code, args, filename
    );

    if (!success) {
        std::cerr << "Syntax errors found in " << filename << std::endl;
        return 1;
    }

    std::cout << "Parsing completed for " << filename << std::endl;
    return 0;
}
