#include "zprompt.hpp"

#include <format>
#include <string>

std::string get_return_code(const Config& config, int return_code) {
    return return_code == 0 ? color_wrap(config.color_return_success, "❯ ")
                            : color_wrap(config.color_return_failure,
                                         std::format("({}) ❯ ", return_code));
}
