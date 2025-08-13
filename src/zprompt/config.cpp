#include "zprompt.hpp"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

namespace {

std::string get_env(const std::string& name) {
    const char* env = getenv(name.c_str());
    if (env == nullptr) {
        return "";
    }
    return env;
}

std::optional<Color> parse_color(const std::optional<std::string>& color_str) {
    return color_str ? magic_enum::enum_cast<Color>(*color_str) : std::nullopt;
}

}  // namespace

Config get_config() {
    fs::path home_path = get_env("HOME");

    const std::vector<std::string> default_pwd_markers = {
        ".git",   ".svn",         "Cargo.toml",
        "go.mod", "package.json", "pyproject.toml",
    };

    const auto default_color_pwd_anchor = Color::magenta;
    const auto default_color_pwd_normal = Color::blue;
    const auto default_color_pwd_error = Color::red;
    const auto default_color_git = Color::green;
    const auto default_color_return_success = Color::green;
    const auto default_color_return_failure = Color::red;
    const auto default_color_ssh = Color::yellow;
    const auto default_color_venv = Color::white;

    try {
        auto config_path = home_path / ".config" / "tools" / "zprompt.toml";

        auto config_file = toml::parse_file(config_path.c_str());

        std::vector<std::string> pwd_markers;
        auto* pwd_markers_arr = config_file["pwd_markers"].as_array();
        if (pwd_markers_arr != nullptr) {
            for (const auto& e : *pwd_markers_arr) {
                if (auto str = e.value<std::string>(); str) {
                    pwd_markers.push_back(*str);
                }
            }
        }

        auto color_str_pwd_anchor =
            config_file["color"]["pwd_anchor"].value<std::string>();
        auto color_str_pwd_normal =
            config_file["color"]["pwd_normal"].value<std::string>();
        auto color_str_pwd_error =
            config_file["color"]["pwd_error"].value<std::string>();
        auto color_str_git = config_file["color"]["git"].value<std::string>();
        auto color_str_return_success =
            config_file["color"]["return_success"].value<std::string>();
        auto color_str_return_failure =
            config_file["color"]["return_failure"].value<std::string>();
        auto color_str_ssh = config_file["color"]["ssh"].value<std::string>();
        auto color_str_venv = config_file["color"]["venv"].value<std::string>();

        auto color_pwd_anchor = parse_color(color_str_pwd_anchor);
        auto color_pwd_normal = parse_color(color_str_pwd_normal);
        auto color_pwd_error = parse_color(color_str_pwd_error);
        auto color_git = parse_color(color_str_git);
        auto color_return_success = parse_color(color_str_return_success);
        auto color_return_failure = parse_color(color_str_return_failure);
        auto color_ssh = parse_color(color_str_ssh);
        auto color_venv = parse_color(color_str_venv);

        return {
            .pwd_markers =
                !pwd_markers.empty() ? pwd_markers : default_pwd_markers,
            .color_pwd_anchor =
                color_pwd_anchor.value_or(default_color_pwd_anchor),
            .color_pwd_normal =
                color_pwd_normal.value_or(default_color_pwd_normal),
            .color_pwd_error =
                color_pwd_error.value_or(default_color_pwd_error),
            .color_git = color_git.value_or(default_color_git),
            .color_return_success =
                color_return_success.value_or(default_color_return_success),
            .color_return_failure =
                color_return_failure.value_or(default_color_return_failure),
            .color_ssh = color_ssh.value_or(default_color_ssh),
            .color_venv = color_venv.value_or(default_color_venv),
        };
    } catch (const std::exception&) {
        return {
            .pwd_markers = default_pwd_markers,
            .color_pwd_anchor = default_color_pwd_anchor,
            .color_pwd_normal = default_color_pwd_normal,
            .color_pwd_error = default_color_pwd_error,
            .color_git = default_color_git,
            .color_return_success = default_color_return_success,
            .color_return_failure = default_color_return_failure,
            .color_ssh = default_color_ssh,
            .color_venv = default_color_venv,
        };
    }
}
