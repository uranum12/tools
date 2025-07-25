#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>
#include <argparse/argparse.hpp>

namespace fs = std::filesystem;

constexpr auto sql_db_init = R"sql(
    CREATE TABLE IF NOT EXISTS histories (
        id INTEGER PRIMARY KEY,
        command TEXT NOT NULL,
        directory TEXT,
        return_code INTEGER,
        time INTEGER,
        UNIQUE(command, directory, return_code)
    );
    CREATE INDEX IF NOT EXISTS idx_filter ON histories(return_code, directory, time);
)sql";

constexpr auto sql_insert = R"sql(
    INSERT OR REPLACE INTO histories (command, directory, return_code, time)
    VALUES (?, ?, ?, ?)
)sql";

constexpr auto sql_insert_command = R"sql(
    INSERT OR IGNORE INTO histories (command) VALUES (?)
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

constexpr auto sql_select = R"sql(
    SELECT command FROM histories
    WHERE return_code = 0 AND directory = ?
    ORDER BY time ASC
)sql";

constexpr auto sql_select_all = R"sql(
    SELECT DISTINCT command FROM histories
    ORDER BY time ASC
)sql";

constexpr auto sql_select_recent = R"sql(
    SELECT DISTINCT command FROM histories
    WHERE return_code = 0
    ORDER BY time DESC
    LIMIT 100;
)sql";

fs::path home_dir() {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        throw std::runtime_error("can't get HOME");
    }
    return fs::path(home);
}

void init(const fs::path& db_dir, const fs::path& db_path) {
    if (!fs::exists(db_dir)) {
        fs::create_directory(db_dir);
    }
    SQLite::Database db(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec(sql_db_init);
}

bool is_command_valid(const std::string& cmd) {
    auto pos = cmd.find_first_not_of(" \t\n\v\f\r");
    return pos != std::string::npos && cmd[pos] != '#';
}

void add(const fs::path& db_path, const std::string& cmd,
         const std::string& dir, int code) {
    if (!is_command_valid(cmd)) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch())
                    .count();

    auto db = SQLite::Database(db_path, SQLite::OPEN_READWRITE);

    SQLite::Statement insert(db, sql_insert);
    insert.bind(1, cmd);
    insert.bind(2, dir);
    insert.bind(3, code);
    insert.bind(4, time);
    insert.exec();
}

void load(const fs::path& db_path, const std::vector<std::string>& cmds) {
    SQLite::Database db(db_path, SQLite::OPEN_READWRITE);

    for (const auto& cmd : cmds) {
        if (is_command_valid(cmd)) {
            SQLite::Statement insert(db, sql_insert_command);
            insert.bind(1, cmd);
            insert.exec();
        }
    }

    db.exec(sql_delete_duplicate);
}

std::vector<std::string> select(const fs::path& db_path) {
    auto current = fs::current_path();

    SQLite::Database db(db_path, SQLite::OPEN_READWRITE);

    SQLite::Statement query(db, sql_select);
    query.bind(1, static_cast<std::string>(current));

    std::vector<std::string> commands;
    while (query.executeStep()) {
        commands.push_back(query.getColumn(0).getString());
    }

    return commands;
}

std::vector<std::string> select_all(const fs::path& db_path) {
    SQLite::Database db(db_path, SQLite::OPEN_READWRITE);

    SQLite::Statement query(db, sql_select_all);

    std::vector<std::string> commands;
    while (query.executeStep()) {
        commands.push_back(query.getColumn(0).getString());
    }

    return commands;
}

std::vector<std::string> select_recent(const fs::path& db_path) {
    SQLite::Database db(db_path, SQLite::OPEN_READWRITE);

    SQLite::Statement query(db, sql_select_recent);

    std::vector<std::string> commands;
    while (query.executeStep()) {
        commands.push_back(query.getColumn(0).getString());
    }

    return commands;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("zhist");
    program.add_description("history command using sqlite");

    argparse::ArgumentParser init_command("init");
    init_command.add_description("initialize database");

    argparse::ArgumentParser add_command("add");
    add_command.add_description("add history to database");
    add_command.add_argument("-c", "--command").help("command").required();
    add_command.add_argument("-d", "--directory").help("directory").required();
    add_command.add_argument("-r", "--return-code")
        .help("return code")
        .scan<'i', int>()
        .required();

    argparse::ArgumentParser load_command("load");
    load_command.add_description("load history file to database");
    load_command.add_argument("filename").help("history file name");

    argparse::ArgumentParser list_command("list");
    list_command.add_description("list history");
    auto& filter_group = list_command.add_mutually_exclusive_group();
    filter_group.add_argument("-a", "--all")
        .help("list all commands")
        .default_value(false)
        .implicit_value(true);
    filter_group.add_argument("-r", "--recent")
        .help("list recent success commands")
        .default_value(false)
        .implicit_value(true);

    program.add_subparser(init_command);
    program.add_subparser(add_command);
    program.add_subparser(load_command);
    program.add_subparser(list_command);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    auto db_dir = home_dir() / ".local/share/zhist";
    auto db_path = db_dir / "zhist.db";

    if (program.is_subcommand_used("init")) {
        init(db_dir, db_path);

        return 0;
    }

    if (program.is_subcommand_used("add")) {
        auto cmd = add_command.get<std::string>("--command");
        auto dir = add_command.get<std::string>("--directory");
        auto ret = add_command.get<int>("--return-code");

        try {
            add(db_path, cmd, dir, ret);
        } catch (const std::exception& e) {
            std::cerr << e.what();
            return 1;
        }

        return 0;
    }

    if (program.is_subcommand_used("load")) {
        auto filename = load_command.get<std::string>("filename");

        std::ifstream ifs(filename);
        if (!ifs) {
            std::cerr << "can't open file" << std::endl;
            return 1;
        }

        std::string line;
        std::vector<std::string> lines;
        while (std::getline(ifs, line)) {
            lines.push_back(line);
        }

        load(db_path, lines);

        return 0;
    }

    if (program.is_subcommand_used("list")) {
        auto is_all = list_command.get<bool>("--all");
        auto is_recent = list_command.get<bool>("--recent");

        auto commands = is_all      ? select_all(db_path)
                        : is_recent ? select_recent(db_path)
                                    : select(db_path);

        for (const auto& command : commands | std::views::reverse) {
            std::cout << command << '\n';
        }
        std::cout << std::flush;

        return 0;
    }

    std::cerr << program;
    return 1;
}
