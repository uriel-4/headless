#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <optional>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <sstream>

struct IfDefRule {
    bool end;
    bool negative;
    bool defining;
    std::string rule;

    std::string toString() {
        if (end) return "#endif " + rule;
        if (defining) {
            if (negative) return "#ifndef " + rule;
            return "#ifdef " + rule;
        } else {
            if (negative) return "#if !" + rule;
            return "#if " + rule;
        }
    }
};

class IfDefTracker : public clang::PPCallbacks {
public:
    explicit IfDefTracker(
        clang::SourceManager &SM,
        std::shared_ptr<std::vector<std::pair<long, IfDefRule>>> rules
    ) : SM(SM), rules(rules) {}

    void Ifdef(clang::SourceLocation Loc, const clang::Token &MacroNameTok, const clang::MacroDefinition &MD) override {
        const auto condition = MacroNameTok.getIdentifierInfo()->getName().str().c_str();
        const IfDefRule rule = { false, false, true, condition };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

    void Ifndef(clang::SourceLocation Loc, const clang::Token &MacroNameTok, const clang::MacroDefinition &MD) override {
        const auto condition = MacroNameTok.getIdentifierInfo()->getName().str().c_str();
        const IfDefRule rule = { false, true, true, condition };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

    void Elifdef(clang::SourceLocation Loc, const clang::Token &MacroNameTok, const clang::MacroDefinition &MD) override {
        const auto condition = MacroNameTok.getIdentifierInfo()->getName().str().c_str();
        const IfDefRule rule = { false, false, true, condition };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

    void Elifndef(clang::SourceLocation Loc, const clang::Token &MacroNameTok, const clang::MacroDefinition &MD) override {
        const auto condition = MacroNameTok.getIdentifierInfo()->getName().str().c_str();
        const IfDefRule rule = { false, true, true, condition };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

    void Elif(clang::SourceLocation Loc, clang::SourceRange ConditionRange, ConditionValueKind ConditionValue, clang::SourceLocation IfLoc) override {
        const auto condition = originalAt(ConditionRange).c_str();
        const IfDefRule rule = { false, true, true, condition };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

    void Else(clang::SourceLocation Loc, clang::SourceLocation IfLoc) override {
        if (rules->size() <= 0) return;
        const auto last = rules->at(rules->size() - 1).second;
        const IfDefRule rule = { last.end, !last.negative, last.defining, last.rule };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

    void Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc) override {
        const IfDefRule rule = { true, 0, 0, "" };
        rules->emplace_back(SM.getFileOffset(SM.getSpellingLoc(Loc)), rule);
    }

private:

    clang::SourceManager &SM;
    clang::LangOptions langOpts;

    std::shared_ptr<std::vector<std::pair<long, IfDefRule>>> rules;

    std::string originalAt(clang::SourceRange range) {
        return clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range), SM, langOpts).str();
    }

};

class ImplementationExtractor : public clang::RecursiveASTVisitor<ImplementationExtractor> {
public:
    explicit ImplementationExtractor(
        clang::Rewriter &R,
        std::shared_ptr<std::vector<std::pair<long, IfDefRule>>> rules,
        clang::SourceManager &SM
    )
        : Rewriter(R), SM(SM), rules(rules) {}

    bool VisitFunctionDecl(clang::FunctionDecl *FD);
    bool VisitVarDecl(clang::VarDecl *VD);
    bool VisitFieldDecl(clang::FieldDecl *VD);
    bool VisitCXXMethodDecl(clang::CXXMethodDecl *MD);

    std::string getModifiedHeader();
    std::string getCppImplementations();

private:

    int replaceOffset = 0;
    clang::Rewriter &Rewriter;
    clang::SourceManager &SM;
    clang::LangOptions langOpts;
    std::ostringstream cppCode;
    std::shared_ptr<std::vector<std::pair<long, IfDefRule>>> rules;

    std::vector<std::pair<std::pair<long, long>, std::string>> replacements;

    std::optional<std::string> getIfdefAt(const long& x) {
        std::optional<IfDefRule> current_rule = std::nullopt;
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
            if (auto *record = llvm::dyn_cast<clang::CXXRecordDecl>(parent)) {
                return true;
            }
            parent = parent->getParent();
        }
        return false;
    }

    std::string originalAt(clang::SourceRange range) {
        return clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(range), SM, langOpts).str();
    }

    std::string originalAt(long start, long end) {
        auto buffer = SM.getBufferData(SM.getMainFileID());
        if (start >= 0 && static_cast<size_t>(start) < buffer.size() && end >= start && static_cast<size_t>(end) < buffer.size()) {
            return std::string(buffer.substr(start, end - start));
        }
        return "";
    }

    char originalAt(long offset) {
        auto buffer = SM.getBufferData(SM.getMainFileID());
        if (offset >= 0 && static_cast<size_t>(offset) < buffer.size()) {
            return buffer[offset];
        }
        return '\0';
    }

    void replace(clang::SourceRange range, clang::StringRef newString) {
        replacements.emplace_back(std::pair{ getStartOffset(range.getBegin()), getEndOffset(range.getEnd()) }, newString);
    }

    long getStartOffset(clang::SourceLocation l) {
        return SM.getFileOffset(SM.getSpellingLoc(l));
    }

    long getEndOffset(clang::SourceLocation l) {
        l = clang::Lexer::getLocForEndOfToken(l, 0, SM, langOpts);
        return SM.getFileOffset(SM.getSpellingLoc(l));
    }

    void replace(long start, long end, clang::StringRef newString) {
        replacements.emplace_back(std::pair{ start, end }, newString);
    }
};

struct ExtractionResult {
    std::string h_code;
    std::string c_code;
};

class ExtractAction : public clang::ASTFrontendAction {
public:

    ExtractAction(std::shared_ptr<ExtractionResult> r): result(r) {}

    void ExecuteAction() override {
        rules = std::make_shared<std::vector<std::pair<long, IfDefRule>>>();
        clang::Preprocessor &PP = getCompilerInstance().getPreprocessor();
        tracker = std::make_unique<IfDefTracker>(getCompilerInstance().getSourceManager(), rules);
        PP.addPPCallbacks(std::move(tracker));
        PP.EnterMainSourceFile();
        clang::Token Tok;
        do {
            PP.Lex(Tok);
        } while (Tok.isNot(clang::tok::eof));
        clang::ASTFrontendAction::ExecuteAction();
    }

    void EndSourceFileAction() override {
        ImplementationExtractor extractor(Rewriter, rules, getCompilerInstance().getSourceManager());
        extractor.TraverseDecl(getCompilerInstance().getASTContext().getTranslationUnitDecl());

        result->h_code = extractor.getModifiedHeader();
        result->c_code = extractor.getCppImplementations();
    }

    bool BeginSourceFileAction(clang::CompilerInstance &CI) override {
        CI.getDiagnostics().setClient(new clang::IgnoringDiagConsumer(), /*ShouldOwn=*/true);
        return clang::ASTFrontendAction::BeginSourceFileAction(CI);
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI,
        llvm::StringRef
    ) override {
        Rewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<clang::ASTConsumer>();
    }

private:

    std::shared_ptr<ExtractionResult> result;
    clang::Rewriter Rewriter;
    std::unique_ptr<IfDefTracker> tracker;
    std::shared_ptr<std::vector<std::pair<long, IfDefRule>>> rules;

};

#endif
