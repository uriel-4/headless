#ifndef GENERATORUNITHANDLER_H
#define GENERATORUNITHANDLER_H

#include "Generator.h"

#include <clang/Tooling/Tooling.h>

class GeneratorUnitHandler : public clang::ASTConsumer {
    Generator* generator;
public:
    GeneratorUnitHandler(Generator* parent);
    void HandleTranslationUnit(clang::ASTContext &Context);
};

#endif //GENERATORUNITHANDLER_H
