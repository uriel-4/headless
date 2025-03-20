#ifndef GENERATOR_H
#define GENERATOR_H

#include <clang/Tooling/Tooling.h>
#include <sstream>

class Generator : public clang::ASTFrontendAction {
public:
    Generator();

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI,
        llvm::StringRef InFile
    );

    void GenerateSource(clang::Decl* d, std::stringstream& s, int pad = 0);
    void GenerateHeader(clang::Decl* d, std::stringstream& s, int pad = 0);
};

#endif //GENERATOR_H
