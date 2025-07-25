#include "zprompt.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <ranges>
#include <string>

namespace fs = std::filesystem;

namespace {

constexpr auto color_anchor = Color::magenta;
constexpr auto color_normal = Color::blue;
constexpr auto color_error = Color::red;

constexpr auto pwd_markers = std::to_array({
    ".git",
    ".svn",
    ".node-version",
    ".python-version",
    ".tool-versions",
    "Cargo.toml",
    "go.mod",
    "package.json",
});

bool has_marker(const fs::path& dir) {
    return std::ranges::any_of(pwd_markers, [&](const char* marker) {
        return fs::exists(dir / marker);
    });
}

bool is_subpath(const fs::path& base, const fs::path& path) {
    auto base_size = std::distance(base.begin(), base.end());
    return std::ranges::equal(base, path | std::views::take(base_size));
}

std::string format_path(const fs::path& home_path, const fs::path& pwd_path) {
    std::string result = color_wrap(color_normal, "~");

    if (home_path == pwd_path) {
        return result;
    }

    auto accumulated_path = home_path;

    for (const auto& part : fs::relative(pwd_path, home_path)) {
        accumulated_path /= part;
        if (has_marker(accumulated_path)) {
            result += color_wrap(color_normal, "/") +
                      color_wrap(color_anchor, part.string());
        } else {
            result += color_wrap(color_normal, "/" + part.string());
        }
    }

    return result;
}

std::string format_path(const fs::path& pwd_path) {
    std::string result;
    auto accumulated_path = fs::path("/");

    for (const auto& part : pwd_path | std::views::drop(1)) {
        accumulated_path /= part;
        if (has_marker(accumulated_path)) {
            result += color_wrap(color_normal, "/") +
                      color_wrap(color_anchor, part.string());
        } else {
            result += color_wrap(color_normal, "/" + part.string());
        }
    }

    return result;
}

}  // namespace

std::string get_current_directory() {
    try {
        auto pwd_path = fs::current_path();

        const auto* home_env = getenv("HOME");
        if (home_env != nullptr && strlen(home_env) > 0) {
            auto home_path = fs::path(home_env);
            if (is_subpath(home_path, pwd_path)) {
                return format_path(home_path, pwd_path);
            }
        }

        return format_path(pwd_path);
    } catch (const fs::filesystem_error& e) {
        return color_wrap(color_error, "unknown");
    }
}
