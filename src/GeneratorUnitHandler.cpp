#include "GeneratorUnitHandler.h"

#include <iostream>

#include "Generator.h"

GeneratorUnitHandler::GeneratorUnitHandler(Generator* parent) {
    this->generator = parent;
}

void GeneratorUnitHandler::HandleTranslationUnit(clang::ASTContext &ctx) {

    for (auto *D : ctx.getTranslationUnitDecl()->decls()) {
        std::stringstream s;
        generator->GenerateHeader(D, s);
        std::cout << "header part: \"\n" << s.str() << "\n\".\n";
        // s.clear();
        // generator->GenerateSource(D, s);
        // std::cout << "source part: \"\n" << s.str() << "\n\".\n";
    }

}