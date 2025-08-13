#include "zprompt.hpp"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <string>

std::string get_venv_status(const Config& config) {
    std::string result;

    if (const char* venv_prompt_env = std::getenv("VIRTUAL_ENV_PROMPT");
        venv_prompt_env != nullptr && std::strlen(venv_prompt_env) > 0) {
        result = venv_prompt_env;
    } else if (const char* venv_env = std::getenv("VIRTUAL_ENV");
               venv_env != nullptr && std::strlen(venv_env) > 0) {
        std::filesystem::path venv_path(venv_env);
        result = std::format("({}) ", venv_path.filename().string());
    } else {
        return "";
    }

    return color_wrap(config.color_venv, result);
}
