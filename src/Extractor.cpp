#include "Extractor.h"
#include <fstream>

bool ImplementationExtractor::VisitFunctionDecl(clang::FunctionDecl *f) {
    if (f->hasBody() && !f->isConstexpr()) {
        auto range = f->getBody()->getSourceRange();

        auto body = originalAt(range);
        std::stringstream s;

        if (f->isStatic()) s << "static ";
        if (f->isConstexpr()) s << "constexpr ";

        s << originalAt(f->getReturnTypeSourceRange());
        s << " ";
        s << f->getQualifiedNameAsString();
        s << "(";
        bool first = true;
        for (const auto &p : f->parameters()) {
            if (!first) {
                s << ", ";
            }
            first = false;
            s << originalAt(p->getTypeSourceInfo()->getTypeLoc().getSourceRange());
            s << " ";
            s << p->getNameAsString();
        }
        s << ") ";
        s << body;
        s << ";\n";

        replace(range, ";");
        cppCode << s.str();

//        clang::SourceRange bodyRange = FD->getBody()->getSourceRange();
//        std::string functionBody = clang::Lexer::getSourceText(
//            clang::CharSourceRange::getCharRange(bodyRange), SM, clang::LangOptions()).str();
//
//        // ✅ Remove function body from header
//        Rewriter.ReplaceText(bodyRange, ";");
//
//        // ✅ Save function implementation for .cpp
//        cppCode << functionBody << "\n";
    }
    return true;
}

bool ImplementationExtractor::VisitVarDecl(clang::VarDecl *decl) {
    if (decl->hasInit() && (!isInsideRecord(decl) || decl->isStaticDataMember()) && !decl->isConstexpr()) {
        auto range = decl->getSourceRange();

        auto type = originalAt(decl->getTypeSourceInfo()->getTypeLoc().getSourceRange());

        std::stringstream h;
        if (!isInsideRecord(decl)) h << "extern ";
        if (decl->isStaticDataMember()) h << "static ";
        h << type;
        h << " " << decl->getNameAsString();

        std::stringstream s;
        auto ifdef = getIfdefAt(SM.getFileOffset(SM.getSpellingLoc(decl->getBeginLoc())));
        if (ifdef) {
            s << *ifdef << "\n";
        }
        s << type;
        s << " " << decl->getQualifiedNameAsString();
        s << " = " << originalAt(decl->getInit()->getSourceRange());
        s << ";\n";
        if (ifdef) {
            s << "#endif\n";
        }

        replace(range, h.str());
        cppCode << s.str();
    }
    return true;
}

bool ImplementationExtractor::VisitFieldDecl(clang::FieldDecl *decl) {
    // if (decl->hasInClassInitializer()) {
    //     auto range = decl->getSourceRange();
    //
    //     replace(range, "");
    // }
    // replace(decl->getTypeSourceInfo()->getTypeLoc().getSourceRange(), "");
    ;
    // const long start = SM.getFileOffset(decl->getTypeSpecStartLoc());
    // const long end = SM.getFileOffset(clang::Lexer::getLocForEndOfToken(decl->getTypeSpecEndLoc(), 0, SM, langOpts));

    // printf("field qualifier %s AND %s\n", decl->getQualifiedNameAsString().c_str(), decl->getTypeSpecEndLoc().printToString(SM).c_str());

        // replace(decl->getSourceRange(), "");
    // if (decl->hasInit() && decl->isStaticDataMember() && !decl->isConstexpr()) {
    //     auto range = decl->getSourceRange();
    //
    //     auto type = originalAt(decl->getTypeSourceInfo()->getTypeLoc().getSourceRange());
    //
    //     std::stringstream h;
    //     if (decl->isStaticDataMember()) h << "static ";
    //     h << type;
    //     h << " " << decl->getNameAsString();
    //
    //     std::stringstream s;
    //     s << type;
    //     s << " " << getLocation(decl);
    //     s << decl->getNameAsString();
    //     s << " = " << originalAt(decl->getInit()->getSourceRange());
    //     s << ";\n";
    //
    //     replace(range, h.str());
    //     cppCode << s.str();
    // }
    return true;
}

bool ImplementationExtractor::VisitCXXMethodDecl(clang::CXXMethodDecl *MD) {
    if (MD->hasBody()) {
//        clang::SourceRange bodyRange = MD->getBody()->getSourceRange();
//        std::string methodBody = clang::Lexer::getSourceText(
//            clang::CharSourceRange::getCharRange(bodyRange), SM, clang::LangOptions()).str();
//
//        // ✅ Remove method body from header
//        Rewriter.ReplaceText(bodyRange, ";");
//
//        // ✅ Save method implementation for .cpp
//        cppCode << MD->getQualifiedNameAsString() << methodBody << "\n";
    }
    return true;
}

std::string ImplementationExtractor::getModifiedHeader() {
    std::string sourceText = SM.getBufferData(SM.getMainFileID()).str();
    for (long i = replacements.size() - 1; i >= 0; --i) {
        const auto &[range, newText] = replacements[i];
        const auto &[start, end] = range;
        sourceText.replace(start, end - start, newText);
    }
    return sourceText;
}

std::string ImplementationExtractor::getCppImplementations() {
    return cppCode.str();
}