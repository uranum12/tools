#include "zprompt.hpp"

#include <format>
#include <string>

namespace {

constexpr auto color_success = Color::green;
constexpr auto color_failure = Color::red;

}  // namespace

std::string get_return_code(int return_code) {
    return return_code == 0
               ? color_wrap(color_success, "❯ ")
               : color_wrap(color_failure, std::format("({}) ❯ ", return_code));
}
