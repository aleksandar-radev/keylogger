#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

bool is_today(const std::string& filename) {
    // Expects filename in format YYYY-MM-DD.txt
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm local;
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char today[16];
    sprintf(today, "%04d-%02d-%02d.txt", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return filename == today;
}

void format_log_file(const std::filesystem::path& path, bool is_today_file) {
    std::ifstream in(path);
    if (!in) return;

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // Check if already formatted (only for non-today files)
    if (!is_today_file && content.rfind("<FORMATTED>", 0) == 0) return;

    std::regex special_key(R"(<[^>]+>)");
    std::string formatted;
    std::smatch match;
    size_t i = 0;
    while (i < content.size()) {
        if (content[i] == '<') {
            std::string sub = content.substr(i);
            if (std::regex_search(sub, match, special_key) && match.position() == 0) {
                std::string key = match.str();
                size_t count = 1;
                size_t j = i + key.size();
                while (content.substr(j, key.size()) == key) {
                    ++count;
                    j += key.size();
                }
                if (count > 1) {
                    formatted += key + "<x" + std::to_string(count) + ">";
                } else {
                    formatted += key;
                }
                i = j;
                continue;
            }
        }
        formatted += content[i];
        ++i;
    }

    std::ofstream out(path, std::ios::trunc);
    if (!is_today_file) {
        out << "<FORMATTED>\n";
    }
    out << formatted;
    out.close();
}

void FormatAllLogs() {
    namespace fs = std::filesystem;
    if (!fs::exists("logs") || !fs::is_directory("logs")) return;
    for (const auto& entry : fs::directory_iterator("logs")) {
        if (!entry.is_regular_file()) continue;
        std::string filename = entry.path().filename().string();
        bool today = is_today(filename);
        format_log_file(entry.path(), today);
    }
}