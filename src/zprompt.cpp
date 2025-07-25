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
        std::cerr << err.what() << std::endl;
        return 1;
    }

    auto ret = program.get<int>("return_code");

    auto ssh_status = get_ssh_status();
    auto cwd = get_current_directory();
    auto git_status = get_git_status();
    auto venv_status = get_venv_status();
    auto return_code = get_return_code(ret);

    std::cout << std::format("{}{}{}\n{}{}", ssh_status, cwd, git_status,
                             venv_status, return_code);

    return 0;
}
