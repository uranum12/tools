#include "zhist.hpp"

#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <sqlite3.h>

namespace fs = std::filesystem;

namespace {

std::string escape_newlines(std::string_view src) {
    constexpr int margin_newline = 10;

    std::string escaped;
    escaped.reserve(src.size() + margin_newline);

    for (char c : src) {
        if (c == '\n') {
            escaped.append("\\n");
        } else if (c == '\t') {
            escaped.append("\\t");
        } else {
            escaped.push_back(c);
        }
    }

    return escaped;
}

}  // namespace

namespace command {

int list(const fs::path& db_path, int recent_num, FilterMode filter_mode,
         ViewMode view_mode) {
    sqlite3* db = nullptr;
    int rc =
        sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }

    std::vector<std::string> cmds;
    switch (filter_mode) {
        case FilterMode::All: {
            cmds = sql::select(db);
        } break;
        case FilterMode::Recent: {
            cmds = sql::select(db, recent_num);
        } break;
        case FilterMode::CurrentPath: {
            auto cwd_path = fs::current_path();

            cmds = sql::select(db, cwd_path);
        } break;
    }

    sqlite3_close(db);

    auto view_no_multiline = std::views::filter([](const std::string& s) {
        return s.find('\n') == std::string::npos;
    });

    switch (view_mode) {
        case ViewMode::Normal: {
            for (const auto& cmd : cmds | std::views::reverse) {
                std::cout << cmd << '\n';
            }
        } break;
        case ViewMode::Escaped: {
            for (const auto& cmd : cmds | std::views::reverse) {
                std::cout << escape_newlines(cmd) << '\n';
            }
        } break;
        case ViewMode::FZF: {
            for (const auto& cmd : cmds | std::views::reverse) {
                std::cout << escape_newlines(cmd) << '\t' << cmd << '\0';
            }
        } break;
        case ViewMode::History: {
            for (const auto& cmd :
                 cmds | std::views::reverse | view_no_multiline) {
                std::cout << cmd << '\n';
            }
        } break;
    }

    std::cout << std::flush;

    return 0;
}

}  // namespace command
