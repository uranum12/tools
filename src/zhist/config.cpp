#include "zhist.hpp"

#include <cstdlib>
#include <filesystem>
#include <string>

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

}  // namespace

Config get_config() {
    fs::path home_path = get_env("HOME");

    const auto default_db_path =
        home_path / ".local" / "share" / "zhist" / "zhist.db";
    const int default_recent_num = 100;

    try {
        auto config_path = home_path / ".config" / "tools" / "zhist.toml";

        auto config_file = toml::parse_file(config_path.string());

        auto db_path = config_file["db_path"].value<std::string>();
        auto recent_num = config_file["recent_num"].value<int>();

        return {
            .db_path = db_path ? fs::path(*db_path) : default_db_path,
            .recent_num = recent_num ? *recent_num : default_recent_num,
        };
    } catch (std::exception& _) {
        return {
            .db_path = default_db_path,
            .recent_num = default_recent_num,
        };
    }
}
