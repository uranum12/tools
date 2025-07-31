#include "zhist.hpp"

#include <filesystem>

#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

namespace command {

int init(const fs::path& db_path) {
    auto db_dir = db_path.parent_path();

    if (!fs::exists(db_dir)) {
        fs::create_directory(db_dir);
    }

    SQLite::Database db(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    sql::init(db);

    return 0;
}

}  // namespace command
