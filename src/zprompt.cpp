#include "zprompt.hpp"

#include <exception>
#include <iostream>

#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("zprompt");
    program.add_description("zsh prompt command");
    program.add_argument("return_code").help("return code").scan<'i', int>();

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
        return 1;
    }

    auto config = get_config();

    auto ret = program.get<int>("return_code");

    auto ssh_status = get_ssh_status(config);
    auto cwd = get_current_directory(config);
    auto git_status = get_git_status(config);
    auto venv_status = get_venv_status(config);
    auto return_code = get_return_code(config, ret);

    std::cout << std::format("{}{}{}\n{}{}", ssh_status, cwd, git_status,
                             venv_status, return_code);

    return 0;
}
