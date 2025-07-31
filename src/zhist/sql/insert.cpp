#include "zhist.hpp"

#include <cstdint>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

constexpr auto sql_insert = R"sql(
    INSERT OR REPLACE INTO histories (command, directory, return_code, time)
    VALUES (?, ?, ?, ?)
)sql";

constexpr auto sql_insert_command = R"sql(
    INSERT OR IGNORE INTO histories (command) VALUES (?)
)sql";

namespace sql {

void insert(SQLite::Database& db, const std::string& cmd,
            const std::string& dir, int code, int64_t time) {
    SQLite::Statement insert(db, sql_insert);
    insert.bind(1, cmd);
    insert.bind(2, dir);
    insert.bind(3, code);
    insert.bind(4, time);
    insert.exec();
}

void insert(SQLite::Database& db, const std::string& cmd) {
    SQLite::Statement insert(db, sql_insert_command);
    insert.bind(1, cmd);
    insert.exec();
}

}  // namespace sql
