#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "IfDefParser.hpp"
#include "utils.hpp"

#include <optional>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>

#include <sstream>

bool is_whitespace(char a) {
    return a == ' ' || a == '\t' || a == '\n' || a == '\r' || a == '\f' || a == '\v';
}

bool isConstQualified(clang::ParmVarDecl *p) {
    return p->getType().isConstQualified() || p->getType().isLocalConstQualified();
}

class ImplementationExtractor : public clang::RecursiveASTVisitor<ImplementationExtractor> {
public:
    explicit ImplementationExtractor(
        std::shared_ptr<std::vector<std::pair<long, IfRule>>> rules,
        std::optional<std::string> pathForLines,
        clang::SourceManager &SM
    )
        : SM(SM), pathForLines(pathForLines), rules(rules) {}

    bool VisitFunctionDecl(clang::FunctionDecl *f) {
        if (f->hasBody() && !f->isConstexpr() && !f->isInlineSpecified()) {
            auto bodyRange = f->getBody()->getSourceRange();
            std::stringstream s;

            auto ifdef = getIfdefAt(getStartOffset(f->getBeginLoc()));
            if (ifdef) {
                s << *ifdef << "\n";
            }
            if (pathForLines) {
                s << "\n#line " << getLineNumber(getStartOffset(f->getBeginLoc())) << " \"" << pathForLines.value() << "\"\n";
            }
            if (auto* funcTemplate = f->getDescribedFunctionTemplate()) {
                s << originalAt(funcTemplate->getBeginLoc(), funcTemplate->getTemplateParameters()->getRAngleLoc()) << "\n";
            }
            if (f->getReturnType().isConstQualified()) {
                s << "const ";
            }
            s << originalAt(f->getReturnTypeSourceRange());
            s << " ";
            s << f->getQualifiedNameAsString();
            s << "(";
            bool first = true, addedLines = false;
            for (const auto &p : f->parameters()) {
                if (!first) {
                    s << ", ";
                }
                first = false;
                auto range = p->getSourceRange();
                long start = getStartOffset(range.getBegin());
                long end = getEndOffset(range.getEnd());
                if (p->hasDefaultArg()) {
                    end = getEndOffset(p->getLocation());
                }
                if (pathForLines && getLineNumber(start) != getLineNumber(getStartOffset(f->getBeginLoc()))) {
                    s << "\n#line " << getLineNumber(start) << " \"" << pathForLines.value() << "\"\n\t";
                    addedLines = true;
                }
                s << originalAt(start, end);
            }
            if (addedLines && pathForLines) {
                s << "\n";
            }
            s << ") ";
            if (f->getFunctionType()->isConst()) {
                s << "const ";
            }
            pushOriginal(s, bodyRange);
            s << ";\n";
            if (ifdef) {
                s << "#endif\n";
            }

            long idx = getStartOffset(bodyRange.getBegin()) - 1;
            while (idx >= 0 && is_whitespace(originalAt(idx))) { idx--; }
            replace(idx + 1, getEndOffset(bodyRange.getEnd()), originalAt(getEndOffset(bodyRange.getEnd())) == ';' ? "" : ";");
            cppCode << s.str();
        }
        return true;
    }

    bool VisitVarDecl(clang::VarDecl *decl) {
        bool hasInit = decl->hasInit();
        if (!llvm::isa<clang::ParmVarDecl>(decl) && (!isInsideRecord(decl) || decl->isStaticDataMember()) && !decl->isConstexpr() && !decl->getType()->isUndeducedAutoType()) {
            bool customTypeRange = false;
            clang::SourceRange typeRange, initRange;
            if (!hasInit) {
                auto eq = clang::Lexer::findLocationAfterToken(decl->getLocation(), clang::tok::equal, SM, langOpts, true);
                hasInit = getEndOffset(eq) != 0;
                if (hasInit) {
                    initRange = { eq, decl->getEndLoc() };
                    typeRange = { decl->getBeginLoc(), decl->getLocation() };
                }
            } else {
                initRange = decl->getInit()->getSourceRange();
                typeRange = decl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
                if (!typeRange.isValid()) {
                    const clang::VarDecl* first = decl;
                    if (llvm::isa<clang::CXXRecordDecl>(decl->getDeclContext())) {
                        const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl->getDeclContext());
                        if (record && record->hasDefinition()) {
                            auto myStart = decl->getBeginLoc();
                            for (const auto* sibling : record->decls()) {
                                if (sibling == decl) break;
                                if (const auto* field = llvm::dyn_cast<clang::VarDecl>(sibling)) {
                                    if (field->getBeginLoc() == myStart) {
                                        first = field;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    customTypeRange = true;
                    typeRange = { first->getSourceRange().getBegin(), first->getLocation().getLocWithOffset(-1) };
                }
            }

            if (hasInit) {
                auto eq = clang::Lexer::findNextToken(decl->getLocation(), SM, langOpts);
                if (eq->getKind() == clang::tok::equal) {
                    if (!isInsideRecord(decl)) {
                        auto start = getStartOffset(decl->getSourceRange().getBegin());
                        replace(start, start, "extern ");
                    }
                    if (decl->getType()->getContainedAutoType()) {
                        replace(typeRange, decl->getType().getAsString());
                    }
                    replace(getEndOffset(decl->getLocation()), getEndOffset(initRange.getEnd()), "");
                }

                std::stringstream s;
                auto ifdef = getIfdefAt(getStartOffset(decl->getBeginLoc()));
                if (ifdef) {
                    s << *ifdef << "\n";
                }
                if (pathForLines) {
                    s << "\n#line " << getLineNumber(getStartOffset(decl->getBeginLoc())) << " \"" << pathForLines.value() << "\"\n";
                }
                std::string type;
                if (decl->getType()->getContainedAutoType()) {
                    type = decl->getType().getAsString();
                } else {
                    type = originalAt(typeRange);
                    auto staticInType = type.find("static ");
                    if (staticInType != std::string::npos) {
                        type.replace(staticInType, 7, "");
                    }
                }
                s << type;
                s << " " << decl->getQualifiedNameAsString();
                s << " = ";
                pushOriginal(s, initRange);
                s << ";\n";
                if (ifdef) {
                    s << "#endif\n";
                }

                cppCode << s.str();
            }
        }
        return true;
    }

    void pushOriginal(std::stringstream& s, const clang::SourceRange& range) {
        if (!pathForLines) {
            s << originalAt(range);
            return;
        }

        std::string original = originalAt(range);
        long startOffset = getStartOffset(range.getBegin());
        long firstLineNumber = getLineNumber(startOffset);
        long index = 0;
        while (index != std::string::npos && index < original.length()) {
            long nextLineIndex = original.find_first_of('\n', index);
            long lineEndIndex = nextLineIndex == std::string::npos ? original.length() : nextLineIndex - index;
            auto line = original.substr(index, lineEndIndex);
            if (firstLineNumber != getLineNumber(startOffset + index + 1)) {
                s << "\n#line " << getLineNumber(startOffset + index + 1) << " \"" << pathForLines.value() << "\"\n";
            }
            s << line;
            if (nextLineIndex == std::string::npos) break;
            index = nextLineIndex + 1;
        }
    }

    long getOriginalOffset(long modifiedOffset) {
        long cumulativeDelta = 0;
        for (const auto &r : replacements) {
            long origStart = r.first.first;
            long origEnd   = r.first.second;
            long origLength = origEnd - origStart;
            long newLength  = static_cast<long>(r.second.size());
            long delta = newLength - origLength;
            long modStart = origStart + cumulativeDelta;
            long modEnd = modStart + newLength;

            if (modifiedOffset >= modStart && modifiedOffset < modEnd) {
                return origStart;
            }
            if (modifiedOffset >= modEnd) {
                cumulativeDelta += delta;
            } else {
                break;
            }
        }
        return modifiedOffset - cumulativeDelta;
    }

    std::string getModifiedHeader(std::string sourceText) {
        for (long i = replacements.size() - 1; i >= 0; --i) {
            const auto &[range, newText] = replacements[i];
            const auto &[start, end] = range;
            sourceText.replace(start, end - start, newText);
        }

        if (pathForLines) {
            std::stringstream s;
            long lastLineNumber = -1;
            long index = 0;
            while (index != std::string::npos && index < sourceText.length()) {
                long nextLineIndex = sourceText.find_first_of('\n', index);
                long lineEndIndex = nextLineIndex == std::string::npos ? sourceText.length() : nextLineIndex - index;
                auto line = sourceText.substr(index, lineEndIndex);
                long lineNumber = getLineNumber(getOriginalOffset(index + 1));
                if (lastLineNumber != lineNumber && !(line.empty() || trim(line).empty())) {
                    if (index > 0) {
                        s << "\n";
                    }
                    s << "#line " << lineNumber << " \"" << pathForLines.value() << "\"\n";
                }
                s << line;
                if (nextLineIndex == std::string::npos) break;
                index = nextLineIndex + 1;
            }
            return s.str();
        } else {
            return sourceText;
        }
    }

    std::string getCppImplementations() {
        return cppCode.str();
    }

private:

    int replaceOffset = 0;
    clang::SourceManager &SM;
    clang::LangOptions langOpts;
    std::ostringstream cppCode;
    std::optional<std::string> pathForLines;
    std::shared_ptr<std::vector<std::pair<long, IfRule>>> rules;

    std::vector<std::pair<std::pair<long, long>, std::string>> replacements;

    std::optional<std::string> getIfdefAt(const long& x) {
        std::optional<IfRule> current_rule = std::nullopt;
        for (const auto &[offset, rule] : *rules) {
            if (rule.end) {
                if (offset >= x) {
                    return current_rule.has_value() ? std::optional{ current_rule.value().toString() } : std::nullopt;
                }
            } else {
                if (offset <= x) {
                    current_rule = std::optional{ rule };
                } else {
                    return current_rule.has_value() ? std::optional{ current_rule.value().toString() } : std::nullopt;
                }
            }
        }
        return std::nullopt;
    }

    bool isInsideRecord(const clang::Decl *d) {
        const clang::DeclContext *parent = d->getDeclContext();
        while (parent) {
            if (llvm::isa<clang::CXXRecordDecl>(parent)) {
                return true;
            }
            parent = parent->getParent();
        }
        return false;
    }

    std::string originalAt(const clang::SourceRange& range) {
        return clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range), SM, langOpts).str();
    }

    std::string originalAt(long start, long end) {
        auto buffer = SM.getBufferData(SM.getMainFileID());
        if (start >= 0 && static_cast<size_t>(start) < buffer.size() && end >= start && static_cast<size_t>(end) < buffer.size()) {
            return std::string(buffer.substr(start, end - start));
        }
        return "";
    }

    std::string originalAt(const clang::SourceLocation& startLoc, const clang::SourceLocation& endLoc) {
        return originalAt(
            getStartOffset(startLoc),
            getEndOffset(endLoc)
        );
    }

    char originalAt(long offset) {
        auto buffer = SM.getBufferData(SM.getMainFileID());
        if (offset >= 0 && static_cast<size_t>(offset) < buffer.size()) {
            return buffer[offset];
        }
        return '\0';
    }

    long getStartOffset(const clang::SourceLocation& l) {
        return SM.getFileOffset(SM.getSpellingLoc(l));
    }

    long getEndOffset(const clang::SourceLocation& l) {
        const auto& sl = clang::Lexer::getLocForEndOfToken(l, 0, SM, langOpts);
        return SM.getFileOffset(SM.getSpellingLoc(sl));
    }

    long getLineNumber(long offset) {
        return SM.getSpellingLineNumber(SM.getComposedLoc(SM.getMainFileID(), offset));
    }

    void replace(const clang::SourceRange& range, const clang::StringRef& newString) {
        replace(getStartOffset(range.getBegin()), getEndOffset(range.getEnd()), newString);
    }

    void replace(long start, long end, const clang::StringRef& newString) {
        replacements.emplace_back(std::pair{ start, end }, newString);
    }
};

struct ExtractionResult {
    std::string h_code;
    std::string c_code;
};

class ExtractAction : public clang::ASTFrontendAction {
private:
    std::string originalHeaderCode;
    std::shared_ptr<ExtractionResult> result;
    std::shared_ptr<std::vector<std::pair<long, IfRule>>> rules;
    std::optional<std::string> pathForLines;

public:

    ExtractAction(
        std::string originalHeaderCode,
        std::shared_ptr<ExtractionResult> r,
        std::vector<std::pair<long, IfRule>> rules,
        std::optional<std::string> pathForLines
    ):
    originalHeaderCode(std::move(originalHeaderCode)),
    result(std::move(r)),
    rules(std::make_shared<std::vector<std::pair<long, IfRule>>>(rules)),
    pathForLines(pathForLines) {}

    void EndSourceFileAction() override {
        ImplementationExtractor extractor(rules, pathForLines, getCompilerInstance().getSourceManager());
        extractor.TraverseDecl(getCompilerInstance().getASTContext().getTranslationUnitDecl());

        result->h_code = extractor.getModifiedHeader(originalHeaderCode);
        result->c_code = extractor.getCppImplementations();
    }

    bool BeginSourceFileAction(clang::CompilerInstance &CI) override {
        CI.getDiagnostics().setClient(new clang::IgnoringDiagConsumer(), true);
        return clang::ASTFrontendAction::BeginSourceFileAction(CI);
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI,
        llvm::StringRef
    ) override {
        return std::make_unique<clang::ASTConsumer>();
    }
};

#endif
