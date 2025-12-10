#ifndef ZHIST_HPP
#define ZHIST_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <sqlite3.h>

namespace fs = std::filesystem;

struct Config {
    fs::path db_path;
    int recent_num;
};

Config get_config();

inline bool is_command_valid(const std::string& cmd) {
    auto pos = cmd.find_first_not_of(" \t\n\v\f\r");
    return pos != std::string::npos && cmd[pos] != '#';
}

enum class FilterMode : uint8_t {
    All,
    Recent,
    CurrentPath,
};

enum class ViewMode : uint8_t {
    Normal,
    Escaped,
    FZF,
    History,
};

namespace sql {

void init(sqlite3* db);

void delete_duplicate(sqlite3* db);

void insert(sqlite3* db, const std::string& cmd, const std::string& dir,
            int code, int64_t time);
void insert(sqlite3* db, const std::string& cmd);

std::vector<std::string> select(sqlite3* db);
std::vector<std::string> select(sqlite3* db, const fs::path& cwd_path);
std::vector<std::string> select(sqlite3* db, int limit);

}  // namespace sql

namespace command {

int init(const fs::path& db_path);

int add(const fs::path& db_path, const std::string& cmd, const std::string& dir,
        int code);

int load(const fs::path& db_path, const std::string& history_path);

int list(const fs::path& db_path, int recent_num, FilterMode filter_mode,
         ViewMode view_mode);

}  // namespace command

#endif /* end of include guard: ZHIST_HPP */
