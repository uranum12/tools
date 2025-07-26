#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>

namespace fs = std::filesystem;

namespace {

bool is_file(const fs::path& target) {
    return fs::exists(target) && fs::is_regular_file(target);
}

bool is_under(const fs::path& base, const fs::path& target) {
    auto rel = fs::relative(target, base);
    return !rel.empty() && *rel.begin() != "..";
}

}  // namespace

int main() {
    constexpr auto file_num = 7;

    auto cwd = fs::current_path();

    auto input_lines = std::views::istream<std::string>(std::cin) |
                       std::views::transform([](const std::string& line) {
                           return fs::path(line);
                       }) |
                       std::views::filter([&](const fs::path& file) {
                           return is_file(file) && is_under(cwd, file);
                       }) |
                       std::views::transform([&](const fs::path& file) {
                           return fs::relative(file, cwd).string();
                       }) |
                       std::views::take(file_num);

    for (const auto& file : input_lines) {
        std::cout << file << '\n';
    }

    return 0;
}
