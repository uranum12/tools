#include "zhist.hpp"

#include <filesystem>
#include <iostream>

#include <sqlite3.h>

namespace fs = std::filesystem;

namespace command {

int init(const fs::path& db_path) {
    auto db_dir = db_path.parent_path();

    if (!fs::exists(db_dir)) {
        fs::create_directory(db_dir);
    }

    sqlite3* db = nullptr;
    int rc =
        sqlite3_open_v2(db_path.c_str(), &db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }

    sql::init(db);

    sqlite3_close(db);

    return 0;
}

}  // namespace command
