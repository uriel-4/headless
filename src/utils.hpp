#ifndef UTILS_H
#define UTILS_H

#include <optional>
#include <vector>
#include <string>
#include <iostream>

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

void mkdirp(const std::string& path) {
    std::filesystem::create_directories(path);
}

void unlink(const std::string& path) {
    std::filesystem::remove(path);
}

void copy(const std::string& from, const std::string& to) {
    std::filesystem::copy_file(from, to);
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

std::string filename(const std::string& filename) {
    auto idx = filename.find_last_of('.');
    if (idx == std::string::npos) {
        return "";
    }
    return filename.substr(0, idx);
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

#endif