#include "zhist.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include <sqlite3.h>

namespace command {

int load(const fs::path& db_path, const std::string& history_path) {
    std::ifstream ifs(history_path);
    if (!ifs) {
        std::cerr << "can't open file\n";

        return 1;
    }

    sqlite3* db = nullptr;
    int rc =
        sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }

    std::string cmd;
    while (std::getline(ifs, cmd)) {
        if (is_command_valid(cmd)) {
            sql::insert(db, cmd);
        }
    }

    sql::delete_duplicate(db);

    sqlite3_close(db);

    return 0;
}

}  // namespace command
