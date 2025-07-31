#include "zhist.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

namespace command {

int load(const fs::path& db_path, const std::string& history_path) {
    std::ifstream ifs(history_path);
    if (!ifs) {
        std::cerr << "can't open file\n";

        return 1;
    }

    SQLite::Database db(db_path, SQLite::OPEN_READWRITE);

    std::string cmd;
    while (std::getline(ifs, cmd)) {
        if (is_command_valid(cmd)) {
            sql::insert(db, cmd);
        }
    }

    sql::delete_duplicate(db);

    return 0;
}

}  // namespace command
