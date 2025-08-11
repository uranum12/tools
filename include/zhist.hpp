#ifndef ZHIST_HPP
#define ZHIST_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

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

namespace sql {

void init(SQLite::Database& db);

void delete_duplicate(SQLite::Database& db);

void insert(SQLite::Database& db, const std::string& cmd,
            const std::string& dir, int code, int64_t time);
void insert(SQLite::Database& db, const std::string& cmd);

std::vector<std::string> select(SQLite::Database& db);
std::vector<std::string> select(SQLite::Database& db, const fs::path& cwd_path);
std::vector<std::string> select(SQLite::Database& db, int limit);

}  // namespace sql

namespace command {

int init(const fs::path& db_path);

int add(const fs::path& db_path, const std::string& cmd, const std::string& dir,
        int code);

int load(const fs::path& db_path, const std::string& history_path);

int list(const fs::path& db_path, bool is_all, bool is_recent, int recent_num);

}  // namespace command

#endif /* end of include guard: ZHIST_HPP */
