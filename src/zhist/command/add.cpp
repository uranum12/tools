#include "zhist.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

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

        SQLite::Database db(db_path, SQLite::OPEN_READWRITE);

        sql::insert(db, cmd, dir, code, time);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

}  // namespace command
