#include "zprompt.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <ranges>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool has_marker(const fs::path& dir,
                const std::vector<std::string>& pwd_markers) {
    return std::ranges::any_of(pwd_markers, [&](const std::string& marker) {
        return fs::exists(dir / marker);
    });
}

bool is_subpath(const fs::path& base, const fs::path& path) {
    auto base_size = std::distance(base.begin(), base.end());
    return std::ranges::equal(base, path | std::views::take(base_size));
}

std::string format_path(const fs::path& home_path, const fs::path& pwd_path,
                        const Config& config) {
    std::string result = color_wrap(config.color_pwd_normal, "~");

    if (home_path == pwd_path) {
        return result;
    }

    auto accumulated_path = home_path;

    for (const auto& part : fs::relative(pwd_path, home_path)) {
        accumulated_path /= part;
        if (has_marker(accumulated_path, config.pwd_markers)) {
            result += color_wrap(config.color_pwd_normal, "/") +
                      color_wrap(config.color_pwd_anchor, part.string());
        } else {
            result += color_wrap(config.color_pwd_normal, "/" + part.string());
        }
    }

    return result;
}

std::string format_path(const fs::path& pwd_path, const Config& config) {
    std::string result;
    auto accumulated_path = fs::path("/");

    for (const auto& part : pwd_path | std::views::drop(1)) {
        accumulated_path /= part;
        if (has_marker(accumulated_path, config.pwd_markers)) {
            result += color_wrap(config.color_pwd_normal, "/") +
                      color_wrap(config.color_pwd_anchor, part.string());
        } else {
            result += color_wrap(config.color_pwd_normal, "/" + part.string());
        }
    }

    return result;
}

}  // namespace

std::string get_current_directory(const Config& config) {
    try {
        auto pwd_path = fs::current_path();

        const auto* home_env = getenv("HOME");
        if (home_env != nullptr && strlen(home_env) > 0) {
            auto home_path = fs::path(home_env);
            if (is_subpath(home_path, pwd_path)) {
                return format_path(home_path, pwd_path, config);
            }
        }

        return format_path(pwd_path, config);
    } catch (const fs::filesystem_error& e) {
        return color_wrap(config.color_pwd_error, "unknown");
    }
}
