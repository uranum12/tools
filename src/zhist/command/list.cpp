#include "zhist.hpp"

#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include <sqlite3.h>

namespace fs = std::filesystem;

namespace command {

int list(const fs::path& db_path, bool is_all, bool is_recent, int recent_num) {
    sqlite3* db = nullptr;
    int rc =
        sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }

    std::vector<std::string> cmds;
    if (is_all) {
        cmds = sql::select(db);
    } else if (is_recent) {
        cmds = sql::select(db, recent_num);
    } else {
        auto cwd_path = fs::current_path();

        cmds = sql::select(db, cwd_path);
    }

    sqlite3_close(db);

    for (const auto& cmd : cmds | std::views::reverse) {
        std::cout << cmd << '\n';
    }
    std::cout << std::flush;

    return 0;
}

}  // namespace command
