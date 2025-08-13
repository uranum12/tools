#ifndef ZPROMPT_HPP
#define ZPROMPT_HPP

#include <cstdint>
#include <format>
#include <string>
#include <type_traits>
#include <vector>

enum class Color : uint8_t {
    black = 0,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
};

inline std::string color_wrap(Color color, const std::string& str) {
    return std::format("%F{{{}}}{}%f",
                       static_cast<std::underlying_type_t<Color>>(color), str);
}

struct Config {
    std::vector<std::string> pwd_markers;
    Color color_pwd_anchor;
    Color color_pwd_normal;
    Color color_pwd_error;
    Color color_git;
    Color color_return_success;
    Color color_return_failure;
    Color color_ssh;
    Color color_venv;
};

Config get_config();

std::string get_current_directory(const Config& config);
std::string get_git_status(const Config& config);
std::string get_ssh_status(const Config& config);
std::string get_venv_status(const Config& config);
std::string get_return_code(const Config& config, int return_code);

#endif /* end of include guard: ZPROMPT_HPP */
