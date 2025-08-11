#include "zhist.hpp"

#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

namespace command {

int list(const fs::path& db_path, bool is_all, bool is_recent, int recent_num) {
    SQLite::Database db(db_path, SQLite::OPEN_READONLY);

    std::vector<std::string> cmds;
    if (is_all) {
        cmds = sql::select(db);
    } else if (is_recent) {
        cmds = sql::select(db, recent_num);
    } else {
        auto cwd_path = fs::current_path();

        cmds = sql::select(db, cwd_path);
    }

    for (const auto& cmd : cmds | std::views::reverse) {
        std::cout << cmd << '\n';
    }
    std::cout << std::flush;

    return 0;
}

}  // namespace command
