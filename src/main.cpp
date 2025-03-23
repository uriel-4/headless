#include <fstream>

#include "utils.hpp"
#include "IfDefParser.hpp"
#include "Extractor.hpp"

#include <clang/Tooling/Tooling.h>

#include <cxxopts.hpp>

std::string toHeaderToken(const std::string& filename) {
    std::string result = filename;
    for (char& c : result) {
        if (std::isspace(static_cast<unsigned char>(c)) || c == '.') {
            c = '_';
        } else {
            c = std::toupper(static_cast<unsigned char>(c));
        }
    }
    return result;
}

struct Options {
    bool wrap_headers = false;
    bool wrap_headers_add_random = true;
    bool add_lines = false;
    bool incremental = false;
};

std::pair<std::string, std::string> process(
    const std::string& code,
    const std::string& path,
    const std::pair<std::string, std::string>& output_path,
    const Options& options = {}
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
    auto [rules, clearedCode] = parseIfDefs(code);

    auto result = std::make_shared<ExtractionResult>();
    auto action = std::make_unique<ExtractAction>(code, result, rules, options.add_lines ? std::optional{ path } : std::nullopt);
    clang::tooling::runToolOnCodeWithArgs(std::move(action), clearedCode, args, path);
    result->c_code = "#include \"" + output_path.first + "\"\n\n" + result->c_code;
    if (options.wrap_headers) {
        auto token = toHeaderToken(output_path.first);
        if (options.wrap_headers_add_random) {
            token += "_" + std::to_string((rand() % 100000));
        }
        result->h_code = "#ifndef " + token + "\n#define " + token + "\n\n" + result->h_code + "\n\n#endif";
    }
    return { result->h_code, result->c_code };
}

struct CodeFile {
    bool is_source;
    bool generated;
    std::string input;
    std::string output;
    long last_modified;
};

std::string write_sources(const std::vector<CodeFile>& sources) {
    std::stringstream s;
    std::set<std::string> written;
    s << "# Automatically generated with `headless`\n";
    s << "set(SOURCES \n";
    for (const auto &[
        is_source,
        generated,
        input,
        output,
        last_modified
    ] : sources) {
        if (is_source) {
            s << "\t" << output;
        } else if (written.contains(input)) continue;
        if (generated) {
            s << "\t# from \"" << input << "\", last modified: " << last_modified;
            written.emplace(input);
        }
        s << "\n";
    }
    s << ")\n";
    return s.str();
}

std::map<std::string, long> read_sources(const std::string& file) {
    std::map<std::string, long> entries;

    std::regex regex(R"(.*# from \"(.*)\", last modified: (.*)\s*)");
    std::istringstream stream(file);
    std::string line;
    std::smatch match;
    while (std::getline(stream, line)) {
        if (std::regex_match(line, match, regex)) {
            try {
                auto path = match[1].str();
                long last_modified = std::stol(match[2].str());
                entries[path] = last_modified;
            } catch (...) {}
        }
    }

    return entries;
}

long get_last_modified(const std::map<std::string, long>& times, const std::string& path) {
    auto it = times.find(path);
    if (it != times.end())
        return it->second;
    return 0;
}

void sync(
    const std::filesystem::path& dir,

    const std::filesystem::path& from,
    const std::filesystem::path& to,
    const Options& options,

    const std::map<std::string, long>& times,
    std::vector<CodeFile>& out_sources
) {
    for (const auto &[name, is_dir] : read_dir(from)) {
        if (is_dir) {
            sync(dir / name, from / name, to / name, options, times, out_sources);
            continue;
        }
        if (name == ".DS_Store")
            continue;
        if (dir.empty() && name == "sources.cmake")
            continue;

        auto path = dir / name;
        auto ex = ext(name);
        if (ex != "hpp" && ex != "h") {
            if (!exists(to / name) || !options.incremental || get_last_modified(times, path) != get_last_modified(from / name)) {
                if (exists(to / name))
                    unlink(to / name);
                copy(from / name, to / name);
            }

            if ((ex == "c" || ex == "cc" || ex == "cpp") && exists(to / name)) {
                auto last_modified = get_last_modified(from / name);
                out_sources.emplace_back(
                    true, false, "", path, last_modified
                );
            }
        } else {
            auto last_modified = get_last_modified(from / name);
            auto h_file = to / (filename(name) + ".hpp");
            auto c_file = to / (filename(name) + ".cpp");

            auto c_file_from = from / (filename(name) + ".cpp");
            if (!exists(c_file_from)) c_file_from = from / (filename(name) + ".c");
            if (!exists(c_file_from)) c_file_from = from / (filename(name) + ".cc");
            if (exists(c_file_from)) {
                // don't generate, source file is already in `from` folder
                if (!exists(c_file) || !exists(h_file) || !options.incremental || get_last_modified(times, path) != last_modified) {
                    if (exists(h_file)) unlink(h_file);
                    copy(from / name, h_file);
                    out_sources.emplace_back(false, false, path, h_file, last_modified);
                }
                continue;
            }

            if (!exists(c_file) || !exists(h_file) || !options.incremental || get_last_modified(times, path) != last_modified) {
                auto code = read_file(from / name);
                if (!code) continue;

                std::cout << "Generate [" << (dir / (filename(name) + ".hpp")) << ", " << (dir / (filename(name) + ".cpp")) << "] from [" << path << "]; cachedTime=" << get_last_modified(times, path) << ", time=" << last_modified << std::endl;
                auto [h, c] = process(
                    code.value(),
                    path,
                    { dir / (filename(name) + ".hpp"), dir / (filename(name) + ".cpp") },
                    options
                );
                auto last_modified = get_last_modified(from / name);
                write_file(c_file, c);
                out_sources.emplace_back(true, true, path, c_file, last_modified);
                write_file(h_file, h);
                out_sources.emplace_back(false, true, path, h_file, last_modified);
            } else {
                out_sources.emplace_back(true, true, path, c_file, last_modified);
                out_sources.emplace_back(false, true, path, h_file, last_modified);
            }
        }
    }
}

int main(int argc, char **argv) {
    cxxopts::Options options("headless", "Splitting .hpp into .hpp header and .cpp source files");

    options.add_options()
        ("h,help", "Print this help")
        ("t,test", "Test `headless` against directory of examples", cxxopts::value<std::string>())
        ("from", "Specify from which directory to sync files", cxxopts::value<std::string>())
        ("to", "Specify to which directory to sync files", cxxopts::value<std::string>())
        ("g", "Generate `sources.cmake` with a list of source `.cpp` files that were generated")
        ("i", "Sync files incrementally (update only those that were changed). Forces -g")
        ("w,wrap", "Wrap headers around with #ifndef HEADER_XXX; #define HEADER_XXX; ... #endif")
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

            auto add_lines = test == "lines";
            auto wrap_headers = test == "wrap";

            auto start = millis();
            auto [h, c] = process(input.value(), "input.hpp", { "expect.hpp", "expect.cpp" }, { wrap_headers, false, add_lines });
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
                // std::cout << "[C] Result: " << (escape(c) == escape(expected_c.value())) << std::endl << std::endl;

                write_file(test_dir / test / ".result.hpp", h);
                write_file(test_dir / test / ".result.cpp", c);
            }
        }
        return (had_fails ? 1 : 0);
    }

    auto add_lines = !!result.count("lines");
    auto wrap_headers = !!result.count("wrap");
    auto incremental = !!result.count("i");
    auto generate_sources = incremental || !!result.count("g");
    if (result.count("to") && result.count("from")) {
        auto from = std::filesystem::path(result["from"].as<std::string>());
        auto to = std::filesystem::path(result["to"].as<std::string>());

        if (!exists(from)) {
            std::cerr << "headless: Can't find \"" << from << "\"" << std::endl;
            return 1;
        }

        if (!exists(to)) {
            mkdirp(to);
        }

        std::map<std::string, long> sources_times;
        std::vector<CodeFile> sources;
        auto sourcesFile = to / "sources.cmake";
        if (incremental && exists(sourcesFile)) {
            auto content = read_file(sourcesFile);
            if (content) {
                sources_times = read_sources(content.value());
            }
        }

        sync("", from, to, {
            wrap_headers, true, add_lines, incremental
        }, sources_times, sources);

        if (generate_sources) {
            write_file(sourcesFile, write_sources(sources));
        }
        return 0;
    }

    std::cout << options.help() << std::endl;
    return 1;
}
