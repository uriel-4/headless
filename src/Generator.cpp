#include "Generator.h"
#include "GeneratorUnitHandler.h"

#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclFriend.h>

#include <sstream>

Generator::Generator() {

}

std::unique_ptr<clang::ASTConsumer> Generator::CreateASTConsumer(
    clang::CompilerInstance &CI,
    llvm::StringRef InFile
) {
    return std::make_unique<GeneratorUnitHandler>(this);
}

void Generator::GenerateSource(clang::Decl* d, std::stringstream& s, int pad) {

}

std::string recordName(clang::CXXRecordDecl* record) {
    if (record->isClass())
        return "class";
    if (record->isUnion())
        return "union";
    if (record->isStruct())
        return "struct";
    return "???";
}

void newline(std::stringstream& s, int pad) {
    s << "\n";
    for (int i = 0; i < pad; i++) {
        s << "\t";
    }
}

std::string getSourceFile(const clang::SourceRange &range, clang::ASTContext &ctx) {
    if (range.isInvalid()) {
        return "/* original source unavailable */";
    }
    return clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range), ctx.getSourceManager(), {}).str();
}

std::string getSourceFile(const clang::TypeSourceInfo* type, clang::ASTContext &ctx) {
    return getSourceFile(type->getTypeLoc().getSourceRange(), ctx);
}

void Generator::GenerateHeader(clang::Decl* d, std::stringstream &s, int pad) {
    if (auto *record = llvm::dyn_cast<clang::CXXRecordDecl>(d)) {
        if (!record->isCompleteDefinition()) {
            return;
        }
        s << recordName(record) << " " << record->getNameAsString();
        if (record->getNumBases() > 0) {
            s << " : ";
            bool first = true;
            for (auto &base : record->bases()) {
                if (!first)
                    s << ", ";
                first = false;

                if (base.isVirtual()) s << "virtual ";
                switch (base.getAccessSpecifier()) {
                    case clang::AS_public: s << "public "; break;
                    case clang::AS_private: s << "private "; break;
                    case clang::AS_protected: s << "protected "; break;
                }
                s << base.getType().getAsString();
            }
        }
        if (record->hasDefinition()) {
            s << " {";
            for (auto *child : record->decls()) {
                newline(s, pad + 1);
                GenerateHeader(child, s, pad + 1);
            }
            newline(s, pad);
            s << "}";
        }
        s << ";";
        newline(s, pad);
    } else if (auto *namespacedef = llvm::dyn_cast<clang::NamespaceDecl>(d)) {
        s << "namespace " << namespacedef->getNameAsString() << " {";
        bool first = true;
        for (auto *child : namespacedef->decls()) {
            newline(s, pad + 1);
            GenerateHeader(child, s, pad + 1);
        }
        newline(s, pad);
        s << "}";
        newline(s, pad);
    } else if (auto *f = llvm::dyn_cast<clang::FunctionDecl>(d)) {
        if (f->isStatic()) s << "static ";
        if (f->isVirtualAsWritten()) s << "virtual ";
        if (f->isInlineSpecified()) s << "inline ";
        s << f->getReturnType().getAsString() << " " << f->getNameAsString() << " (";
        if (f->getNumParams() > 0) {
            newline(s, pad + 1);
            bool first = true;
            for (auto *param : f->parameters()) {
                if (!first) {
                    s << ",";
                    newline(s, pad + 1);
                }
                first = false;
                s << getSourceFile(param->getTypeSourceInfo(), f->getASTContext());
                s << " " << param->getNameAsString();
                if (param->hasDefaultArg()) {
                    s << " = ";
                    s << getSourceFile(param->getDefaultArg()->getSourceRange(), f->getASTContext());
                }
            }
            newline(s, pad);
        }
        s << ")";
        if (f->isPure()) {
            s << " = 0";
        }
        s << ";";
    } else if (auto *var = llvm::dyn_cast<clang::VarDecl>(d)) {
        if (var->isStaticLocal()) s << "static ";
        if (var->isExternC()) s << "extern ";
        if (var->isConstexpr()) s << "constexpr ";
        if (var->getType().isConstQualified()) s << "const ";
        s << getSourceFile(var->getTypeSourceInfo(), var->getASTContext());
        s << " " << var->getNameAsString();
        if (var->hasInit()) {
            s << " = " << getSourceFile(var->getInit()->getSourceRange(), var->getASTContext());
        }
        s << ";"; // End statement
    } else if (auto *field = llvm::dyn_cast<clang::FieldDecl>(d)) {
        s << getSourceFile(field->getSourceRange(), field->getASTContext());
        if (field->hasInClassInitializer() && field->getInClassInitializer()) {
            s << " = " << getSourceFile(field->getInClassInitializer()->getSourceRange(), field->getASTContext());
        }
        s << ";";
    } else if (auto *access = llvm::dyn_cast<clang::AccessSpecDecl>(d)) {
        switch (access->getAccess()) {
            case clang::AS_public: s << "public:"; break;
            case clang::AS_private: s << "private:"; break;
            case clang::AS_protected: s << "protected:"; break;
        }
    } else if (auto *type = llvm::dyn_cast<clang::TypedefDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else if (auto *typealias = llvm::dyn_cast<clang::TypeAliasDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else if (auto *templaterecord = llvm::dyn_cast<clang::ClassTemplateDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else if (auto *templatefunction = llvm::dyn_cast<clang::FunctionTemplateDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else if (auto *enumdef = llvm::dyn_cast<clang::EnumDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else if (auto *externc = llvm::dyn_cast<clang::LinkageSpecDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else if (auto *frienddef = llvm::dyn_cast<clang::FriendDecl>(d)) {
        // TODO
        s << "<!> TODO: " << d->getDeclKindName();
    } else {
        s << "<!> UNKNOWN: " << d->getDeclKindName();
    }
}