#include "zhist.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

namespace fs = std::filesystem;

constexpr auto sql_select_all = R"sql(
    SELECT DISTINCT command FROM histories
    ORDER BY time ASC
)sql";

constexpr auto sql_select_success = R"sql(
    SELECT command FROM histories
    WHERE return_code = 0 AND directory = ?
    ORDER BY time ASC
)sql";

constexpr auto sql_select_recent = R"sql(
    SELECT DISTINCT command FROM histories
    WHERE return_code = 0
    ORDER BY time DESC
    LIMIT ?;
)sql";

namespace sql {

std::vector<std::string> select(SQLite::Database& db) {
    SQLite::Statement query(db, sql_select_all);

    std::vector<std::string> commands;
    while (query.executeStep()) {
        commands.push_back(query.getColumn(0).getString());
    }

    return commands;
}

std::vector<std::string> select(SQLite::Database& db,
                                const fs::path& cwd_path) {
    SQLite::Statement query(db, sql_select_success);
    query.bind(1, cwd_path.string());

    std::vector<std::string> commands;
    while (query.executeStep()) {
        commands.push_back(query.getColumn(0).getString());
    }

    return commands;
}

std::vector<std::string> select(SQLite::Database& db, int limit) {
    SQLite::Statement query(db, sql_select_recent);
    query.bind(1, limit);

    std::vector<std::string> commands;
    while (query.executeStep()) {
        commands.push_back(query.getColumn(0).getString());
    }

    return commands;
}

}  // namespace sql
