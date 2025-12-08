#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>
#include <argparse/argparse.hpp>
#include <exiv2/exiv2.hpp>

namespace fs = std::filesystem;

namespace {

enum class Mode : uint8_t {
    dry_run,
    copy,
    move,
};

const std::set<std::string> image_exts = {".heic", ".heif", ".jpeg", ".jpg",
                                          ".png"};

std::string to_lower(const std::string& str) {
    auto lower_view = str | std::ranges::views::transform([](unsigned char c) {
                          return static_cast<char>(std::tolower(c));
                      });
    return {lower_view.begin(), lower_view.end()};
}

std::optional<std::tuple<int, int, int>> parse_date(
    const std::string& date_str) {
    if (date_str.size() < 10) {
        return std::nullopt;
    }

    char sep = date_str[4];

    if (sep != ':' && sep != '-') {
        return std::nullopt;
    }

    try {
        int year = std::stoi(date_str.substr(0, 4));
        int month = std::stoi(date_str.substr(5, 2));
        int day = std::stoi(date_str.substr(8, 2));
        return std::make_tuple(year, month, day);
    } catch (...) {
        return std::nullopt;
    }
}

std::string format_date(int year, int month, int day) {
    return fmt::format("{:04d}-{:02d}-{:02d}", year, month, day);
}

fs::path make_next_path(const fs::path& base_path, int num) {
    const auto& parent = base_path.parent_path();
    const auto& stem = base_path.stem().string();
    const auto& ext = base_path.extension().string();

    const auto& filename = fmt::format("{}-{}{}", stem, num, ext);

    return parent / filename;
}

std::vector<std::pair<fs::path, fs::path>> search_target(
    const fs::path& source, const fs::path& output) {
    std::vector<std::pair<fs::path, fs::path>> vec_target;

    for (const auto& entry : fs::directory_iterator(source)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto& img_path = entry.path();
        const auto& img_str = img_path.string();
        const auto& ext = to_lower(img_path.extension().string());

        if (!image_exts.contains(ext)) {
            continue;
        }

        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(img_path);
        image->readMetadata();

        Exiv2::ExifData& exif = image->exifData();
        if (exif.empty()) {
            fmt::print(fmt::fg(fmt::terminal_color::red),
                       "Can't find exif data: {}\n", img_str);
            continue;
        }

        auto it = exif.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
        if (it == exif.end()) {
            fmt::print(fmt::fg(fmt::terminal_color::red),
                       "Can't find DateTimeOriginal: {}\n", img_str);
            continue;
        }

        auto date_str = it->value().toString();
        auto date_opt = parse_date(date_str);

        if (!date_opt.has_value()) {
            fmt::print(fmt::fg(fmt::terminal_color::red),
                       "Can't parse date: {}\n", img_str);
            continue;
        }

        auto [year, month, day] = *date_opt;
        auto output_path =
            output / format_date(year, month, day) / img_path.filename();

        const auto base_path = output_path;
        for (int num = 1; fs::exists(output_path); num++) {
            fmt::print(fmt::fg(fmt::terminal_color::yellow),
                       "File already exists: {}\n", output_path.string());

            output_path = make_next_path(base_path, num);
        }

        vec_target.emplace_back(img_path, output_path);
    }

    return vec_target;
}

}  // namespace

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("imgsort");
    program.add_description("Sort images by date.");

    program.add_argument("-s", "--source")
        .metavar("path")
        .help("source path. default: current");
    program.add_argument("-o", "--output")
        .metavar("path")
        .help("output path. default: source");

    auto& run_group = program.add_mutually_exclusive_group();
    run_group.add_argument("-n", "--dry-run")
        .default_value(false)
        .implicit_value(true)
        .help("Don't move/copy");
    run_group.add_argument("-c", "--copy")
        .default_value(false)
        .implicit_value(true)
        .help("Make copy");
    run_group.add_argument("-m", "--move")
        .default_value(false)
        .implicit_value(true)
        .help("[default] Move");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n' << program;
        return 1;
    }

    auto mode = program.get<bool>("--dry-run") ? Mode::dry_run
                : program.get<bool>("--copy")  ? Mode::copy
                                               : Mode::move;

    try {
        auto source = program.is_used("--source")
                          ? fs::canonical(program.get<std::string>("--source"))
                          : fs::current_path();

        auto output =
            program.is_used("--output")
                ? fs::weakly_canonical(program.get<std::string>("--output"))
                : source;

        auto vec_target = search_target(source, output);

        switch (mode) {
            case Mode::dry_run: {
                for (const auto& [path_s, path_o] : vec_target) {
                    std::cout << path_s << " " << path_o << "\n";
                }
            } break;
            case Mode::copy: {
                for (const auto& [path_s, path_o] : vec_target) {
                    auto parent = path_o.parent_path();
                    if (!fs::exists(parent)) {
                        fs::create_directories(parent);
                    }

                    fs::copy_file(path_s, path_o);

                    fmt::print(fmt::fg(fmt::terminal_color::green),
                               "Copied file: {}\n", path_o.string());
                }
            } break;
            case Mode::move: {
                for (const auto& [path_s, path_o] : vec_target) {
                    auto parent = path_o.parent_path();
                    if (!fs::exists(parent)) {
                        fs::create_directories(parent);
                    }

                    fs::rename(path_s, path_o);

                    fmt::print(fmt::fg(fmt::terminal_color::green),
                               "Moved file: {}\n", path_o.string());
                }
            } break;
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
