#include <exception>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("clean_ds");

    program.add_description("search/delete MacOS hidden metafiles.");

    program.add_argument("path")
        .nargs(argparse::nargs_pattern::optional)
        .default_value("")
        .help("target directory");

    program.add_argument("-d", "--delete")
        .implicit_value(true)
        .default_value(false)
        .help("delete all files.");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        return 1;
    }

    try {
        auto path_str = program.get<std::string>("path");
        auto path =
            path_str.empty() ? fs::current_path() : fs::canonical(path_str);

        auto is_delete = program.get<bool>("--delete");

        auto ds_store = std::regex(R"(^\.DS_Store$)");
        auto apple_double = std::regex(R"(^\._.*)");

        std::vector<fs::path> vec_match;

        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string filename = entry.path().filename().string();

            if (std::regex_search(filename, ds_store) ||
                std::regex_search(filename, apple_double)) {
                vec_match.push_back(entry.path());
            }
        }

        for (const auto& p : vec_match) {
            std::cout << p.string() << '\n';
        }

        if (is_delete) {
            for (const auto& p : vec_match) {
                fs::remove(p);
                std::cout << "remove: " << p.string() << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
