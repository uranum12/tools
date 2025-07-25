#include "zprompt.hpp"

#include <pwd.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <string>

namespace {

constexpr auto color_ssh = Color::yellow;

std::string get_username() {
    struct passwd* pw = getpwuid(geteuid());
    return pw != nullptr && pw->pw_name != nullptr ? pw->pw_name : "unknown";
}

std::string get_hostname() {
    constexpr size_t buf_size = 256;
    std::array<char, buf_size> hostname = {};
    return gethostname(hostname.data(), buf_size) == 0 ? hostname.data()
                                                       : "unknown";
}

}  // namespace

std::string get_ssh_status() {
    std::string result;

    const char* ssh_env = std::getenv("SSH_CONNECTION");
    if (ssh_env != nullptr && strlen(ssh_env) > 0) {
        auto user = get_username();
        auto host = get_hostname();

        result = color_wrap(color_ssh, user) + "@" +
                 color_wrap(color_ssh, host) + ":";
    }

    return result;
}
