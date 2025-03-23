#ifndef IFDEFPARSER_H
#define IFDEFPARSER_H

#include <iostream>
#include <regex>
#include <sstream>

struct IfRule {
    bool end;
    std::string value;

    [[nodiscard]] std::string toString() const {
        return "#if " + value;
    }
};


std::string stripComments(std::string code) {
    enum class State {
        Normal,
        LineComment,
        BlockComment
    };

    State state = State::Normal;
    size_t i = 0;
    while (i < code.length()) {
        if (state == State::Normal) {
            if (code[i] == '/' && i + 1 < code.length()) {
                if (code[i + 1] == '/') {
                    state = State::LineComment;
                    code[i] = ' ';
                    ++i;
                    code[i] = ' ';
                } else if (code[i + 1] == '*') {
                    state = State::BlockComment;
                    code[i] = ' ';
                    ++i;
                    code[i] = ' ';
                }
            }
        } else if (state == State::LineComment) {
            if (code[i] == '\n') {
                state = State::Normal;
            } else {
                code[i] = ' ';
            }
        } else if (state == State::BlockComment) {
            if (code[i] == '*' && i + 1 < code.length() && code[i + 1] == '/') {
                code[i] = ' ';
                ++i;
                code[i] = ' ';
                state = State::Normal;
            } else {
                if (code[i] != '\n') {
                    code[i] = ' ';
                }
            }
        }
        ++i;
    }

    return code;
}

std::string notAccumulated(
    const std::vector<std::string>& accumulatedConditions,
    const std::string& but
) {
    std::stringstream ss;
    ss << "!";
    if (accumulatedConditions.size() > 1)
        ss << "(";
    bool first = true;
    for (auto c : accumulatedConditions) {
        if (!first) {
            ss << " || ";
        }
        first = false;
        ss << c;
    }
    if (accumulatedConditions.size() > 1)
        ss << ")";
    if (!but.empty()) {
        ss << " && " << but;
    }
    return ss.str();
}

std::pair<
    std::vector<std::pair<long, IfRule>>,
    std::string
> parseIfDefs(std::string code) {
    code = stripComments(code);

    std::vector<std::string> accumulatedConditions;
    std::vector<std::pair<long, IfRule>> rules;
    std::vector<std::pair<std::pair<long, long>, std::string>> replacements;

    std::regex directiveRegex(R"(^\s*(#endif|#else|(#if|#ifdef|#ifndef|#elif|#elifdef|#elifndef))\b\s*(.*))");

    std::istringstream stream(code);
    std::string line;
    long offset = 0;

    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_match(line, match, directiveRegex)) {
            auto start = offset + match.position(0);
            auto end = start + match.length(0);
            code.replace(start, end - start, std::string(end - start, ' '));

            auto dir = match[1].str();
            if (dir == "#if") {
                auto condition = match[3].str();
                rules.emplace_back(start, IfRule{ false, condition });
                accumulatedConditions.push_back(condition);
            } else if (dir == "#ifdef") {
                auto condition = "defined(" + match[3].str() + ")";
                rules.emplace_back(start, IfRule{ false, condition });
                accumulatedConditions.push_back(condition);
            } else if (dir == "#ifndef") {
                auto condition = "!defined(" + match[3].str() + ")";
                rules.emplace_back(start, IfRule{ false, condition });
                accumulatedConditions.push_back(condition);
            } else if (dir == "#elif") {
                auto condition = match[3].str();
                rules.emplace_back(start, IfRule{ false, notAccumulated(accumulatedConditions, condition) });
                accumulatedConditions.push_back(condition);
            } else if (dir == "#elifdef") {
                auto condition = "defined(" + match[3].str() + ")";
                rules.emplace_back(start, IfRule{ false, notAccumulated(accumulatedConditions, condition) });
                accumulatedConditions.push_back(condition);
            } else if (dir == "#elifndef") {
                auto condition = "!defined(" + match[3].str() + ")";
                rules.emplace_back(start, IfRule{ false, notAccumulated(accumulatedConditions, condition) });
                accumulatedConditions.push_back(condition);
            } else if (dir == "#else") {
                rules.emplace_back(start, IfRule{ false, notAccumulated(accumulatedConditions, "") });
            } else if (dir == "#endif") {
                rules.emplace_back(start, IfRule{ true, "" });
                accumulatedConditions.clear();
            }
        }
        offset += line.size() + 1;
    }

    return { rules, code };
}

#endif