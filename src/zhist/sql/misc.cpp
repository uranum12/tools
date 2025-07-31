#include "zhist.hpp"

#include <SQLiteCpp/SQLiteCpp.h>

constexpr auto sql_init_table = R"sql(
    CREATE TABLE IF NOT EXISTS histories (
        id INTEGER PRIMARY KEY,
        command TEXT NOT NULL,
        directory TEXT,
        return_code INTEGER,
        time INTEGER,
        UNIQUE(command, directory, return_code)
    );
)sql";

constexpr auto sql_init_index = R"sql(
    CREATE INDEX IF NOT EXISTS idx_filter ON histories(return_code, directory, time);
)sql";

constexpr auto sql_delete_duplicate = R"sql(
    WITH duplicated_commands AS (
        SELECT command
        FROM histories
        WHERE directory IS NULL AND return_code IS NULL
        GROUP BY command
        HAVING COUNT(*) > 1
    ),
    histories_to_keep AS (
        SELECT MIN(id) AS id
        FROM histories
        WHERE directory IS NULL AND return_code IS NULL
          AND command IN (SELECT command FROM duplicated_commands)
        GROUP BY command
    ),
    histories_to_delete AS (
        SELECT id
        FROM histories
        WHERE directory IS NULL AND return_code IS NULL
          AND command IN (SELECT command FROM duplicated_commands)
          AND id NOT IN (SELECT id FROM histories_to_keep)
    )
    DELETE FROM histories
    WHERE id IN (SELECT id FROM histories_to_delete);
)sql";

namespace sql {

void init(SQLite::Database& db) {
    db.exec(sql_init_table);
    db.exec(sql_init_index);
}

void delete_duplicate(SQLite::Database& db) {
    db.exec(sql_delete_duplicate);
}

}  // namespace sql
