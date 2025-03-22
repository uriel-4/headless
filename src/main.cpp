#include <fstream>

#include "Extractor.h"

#include <clang/Tooling/Tooling.h>
#include <iostream>

#include <cxxopts.hpp>

std::optional<std::string> read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return std::nullopt;
    }
    std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return { content };
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (file) file << content;
}

std::vector<std::pair<std::string, bool>> read_dir(const std::string& path) {
    std::vector<std::pair<std::string, bool>> entries;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        entries.emplace_back(
            entry.path().filename().string(),
            entry.is_directory()
        );
    }
    return entries;
}

long get_last_modified(const std::string& path) {
    auto ftime = std::filesystem::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
    );
    return std::chrono::system_clock::to_time_t(sctp);
}

bool exists(const std::string& path) {
    return std::filesystem::exists(path);
}

void unlink(const std::string& path) {
    std::filesystem::remove(path);
}

long long millis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

const char separator = std::filesystem::path::preferred_separator;

std::string ext(const std::string& filename) {
    auto idx = filename.find_last_of('.');
    if (idx == std::string::npos) {
        return "";
    }
    return filename.substr(idx + 1);
}

const char* ws = " \t\n\r\f\v";
inline std::string& rtrim(std::string& s, const char* t = ws) {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

inline std::string& ltrim(std::string& s, const char* t = ws) {
    s.erase(0, s.find_first_not_of(t));
    return s;
}

inline std::string& trim(std::string& s, const char* t = ws) {
    return ltrim(rtrim(s, t), t);
}

#include <regex>
std::string escape(const std::string& s) {
    auto replaced = std::regex_replace(s, std::regex{ "\\s+" }, " ");
    return trim(replaced);
}

std::pair<std::string, std::string> process(
    const std::string& code,
    const std::string& path,
    const std::pair<std::string, std::string>& output_path,

    bool add_lines
) {
    const std::vector<std::string> args = {
        "-std=c++17",
        "-fsyntax-only",
        "-ffreestanding",
        "-Wno-everything",
        "-Wno-error",
        "-nostdinc",
        "-nostdinc++",
        "-E"
    };
    auto result = std::make_shared<ExtractionResult>();
    auto action = std::make_unique<ExtractAction>(result);
    clang::tooling::runToolOnCodeWithArgs(std::move(action), code, args, path);
    result->c_code = "#include \"" + output_path.first + "\"\n\n" + result->c_code;
    return { result->h_code, result->c_code };
}

int main(int argc, char **argv) {
    cxxopts::Options options("headless", "Splitting .hpp into .hpp header and .cpp source files");

    options.add_options()
        ("h,help", "Print this help")
        ("t,test", "Test `headless` against directory of examples", cxxopts::value<std::string>())
        ("from", "Specify from which directory to sync files", cxxopts::value<std::string>())
        ("to", "Specify to which directory to sync files", cxxopts::value<std::string>())
        ("i", "Sync files incrementally (update only those that were changed). Will generate `.snapshot.txt` file in output directory to track files last modified date.")
        ("g", "Generate `sources.txt` with a list of source .cpp files that were generated")
        ("w", "Wrap headers around with #ifndef HEADER_XXX; #define HEADER_XXX; ... #endif")
        ("l,lines", "Add #line in outputs for debug")
        ;
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("test")) {
        bool had_fails = false;
        auto test_dir = std::filesystem::path(result["test"].as<std::string>());
        for (const auto [test, is_dir] : read_dir(test_dir)) {
            if (!is_dir) continue;
            remove(test_dir / test / ".result.hpp");
            remove(test_dir / test / ".result.cpp");

            auto input = read_file(test_dir / test / "input.hpp");
            auto expected_h = read_file(test_dir / test / "expect.hpp");
            auto expected_c = read_file(test_dir / test / "expect.cpp");

            if (!input || !expected_c || !expected_h) continue;
            std::cout << "Test \"" << test << "\": ";

            auto start = millis();
            auto [h, c] = process(input.value(), "input.hpp", { "expect.hpp", "expect.cpp" }, false);
            auto duration = millis() - start;

            bool success = escape(h) == escape(expected_h.value()) && escape(c) == escape(expected_c.value());
            if (success) {
                std::cout << "✅ Success (" << duration << "ms)" << std::endl;
            } else {
                std::cout << "❌ Failed (" << duration << "ms)" << std::endl;
                had_fails = true;

                // std::cout << "[H] Expected: " << escape(expected_h.value()) << std::endl;
                // std::cout << "[H] Got:      " << escape(h) << std::endl;
                // std::cout << "[H] Result: " << (escape(h) == escape(expected_h.value())) << std::endl << std::endl;
                //
                // std::cout << "[C] Expected: " << escape(expected_c.value()) << std::endl;
                // std::cout << "[C] Got:      " << escape(c) << std::endl;
                // std::cout << "[H] Result: " << (escape(c) == escape(expected_c.value())) << std::endl << std::endl;

                write_file(test_dir / test / ".result.hpp", h);
                write_file(test_dir / test / ".result.cpp", c);
            }
        }
        return (had_fails ? 1 : 0);
    }


    return 0;
}
