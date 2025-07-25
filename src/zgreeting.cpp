#include <array>
#include <chrono>
#include <random>

#include <fmt/color.h>
#include <fmt/core.h>

constexpr auto terminal_colors = std::to_array({
    fmt::terminal_color::red,
    fmt::terminal_color::green,
    fmt::terminal_color::yellow,
    fmt::terminal_color::blue,
    fmt::terminal_color::magenta,
    fmt::terminal_color::cyan,
    fmt::terminal_color::white,
});

constexpr auto color_yellow = fmt::fg(fmt::terminal_color::yellow);
constexpr auto color_red = fmt::fg(fmt::terminal_color::red);
constexpr auto color_blue = fmt::fg(fmt::terminal_color::blue);

int main() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto seed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    std::mt19937 rng(static_cast<int>(seed));
    std::uniform_int_distribution<int> dist(0, terminal_colors.size() - 1);

    auto color_telescope = fmt::fg(terminal_colors.at(dist(rng)));
    auto color_zsh = fmt::fg(terminal_colors.at(dist(rng)));

    auto lines = std::to_array({
        "     " + fmt::format(color_telescope, "______________") +
            "            *        /     " + fmt::format(color_yellow, "o\n"),
        "  ==" + fmt::format(color_telescope, "(_____(o(___(_()") +
            "                   /\n",
        R"(          /|\             )" + fmt::format(color_red, "o") +
            "      |    *            *\n",
        R"(         / | \)" + fmt::format(color_zsh, "┌─┐┌─┐┬ ┬") +
            "         -+-        +\n",
        "        /  |  " + fmt::format(color_zsh, "┌─┘└─┐├─┤") +
            "          |              " + fmt::format(color_blue, "o\n"),
        "       /      " + fmt::format(color_zsh, "└─┘└─┘┴ ┴") +
            "     *          ~+\n",
    });

    for (const auto& line : lines) {
        fmt::print("{}", line);
    }

    return 0;
}
