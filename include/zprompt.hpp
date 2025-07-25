#ifndef ZPROMPT_HPP
#define ZPROMPT_HPP

#include <cstdint>
#include <format>
#include <string>
#include <type_traits>

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

std::string get_current_directory();
std::string get_git_status();
std::string get_ssh_status();
std::string get_venv_status();
std::string get_return_code(int return_code);

#endif /* end of include guard: ZPROMPT_HPP */
