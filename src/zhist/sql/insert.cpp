#include "zhist.hpp"

#include <cstdint>
#include <string>

#include <sqlite3.h>

constexpr auto sql_insert = R"sql(
    INSERT OR REPLACE INTO histories (command, directory, return_code, time)
    VALUES (?, ?, ?, ?)
)sql";

constexpr auto sql_insert_command = R"sql(
    INSERT OR IGNORE INTO histories (command) VALUES (?)
)sql";

namespace sql {

void insert(sqlite3* db, const std::string& cmd, const std::string& dir,
            int code, int64_t time) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return;
    }

    sqlite3_bind_text(stmt, 1, cmd.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dir.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, code);
    sqlite3_bind_int64(stmt, 4, time);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);
}

void insert(sqlite3* db, const std::string& cmd) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql_insert_command, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return;
    }

    sqlite3_bind_text(stmt, 1, cmd.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);
}

}  // namespace sql
