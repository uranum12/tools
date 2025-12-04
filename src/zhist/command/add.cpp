#include "zhist.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include <sqlite3.h>

namespace fs = std::filesystem;

using ms = std::chrono::milliseconds;
using std::chrono::duration_cast;

namespace command {

int add(const fs::path& db_path, const std::string& cmd, const std::string& dir,
        int code) {
    try {
        if (!is_command_valid(cmd)) {
            return 0;
        }

        auto now = std::chrono::system_clock::now();
        auto time = duration_cast<ms>(now.time_since_epoch()).count();

        sqlite3* db = nullptr;
        int rc = sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READWRITE,
                                 nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
            sqlite3_close(db);
            return 1;
        }

        sql::insert(db, cmd, dir, code, time);

        sqlite3_close(db);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

}  // namespace command
