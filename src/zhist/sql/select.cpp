#include "zhist.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <sqlite3.h>

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

std::vector<std::string> select(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql_select_all, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::vector<std::string> commands;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto* txt = sqlite3_column_text(stmt, 0);
        if (txt == nullptr) {
            continue;
        }
        commands.emplace_back(reinterpret_cast<const char*>(txt));
    }

    sqlite3_finalize(stmt);

    return commands;
}

std::vector<std::string> select(sqlite3* db, const fs::path& cwd_path) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql_select_success, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    std::string dir = cwd_path.string();
    sqlite3_bind_text(stmt, 1, dir.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<std::string> commands;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto* txt = sqlite3_column_text(stmt, 0);
        if (txt == nullptr) {
            continue;
        }
        commands.emplace_back(reinterpret_cast<const char*>(txt));
    }

    sqlite3_finalize(stmt);

    return commands;
}

std::vector<std::string> select(sqlite3* db, int limit) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql_select_recent, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return {};
    }

    sqlite3_bind_int(stmt, 1, limit);

    std::vector<std::string> commands;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto* txt = sqlite3_column_text(stmt, 0);
        if (txt == nullptr) {
            continue;
        }
        commands.emplace_back(reinterpret_cast<const char*>(txt));
    }

    sqlite3_finalize(stmt);

    return commands;
}

}  // namespace sql
