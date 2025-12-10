#include "zhist.hpp"

#include <exception>
#include <iostream>
#include <string>

#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("zhist");
    program.add_description("history command using sqlite");

    argparse::ArgumentParser init_command("init");
    init_command.add_description("initialize database");

    argparse::ArgumentParser add_command("add");
    add_command.add_description("add history to database");
    add_command.add_argument("-c", "--command").help("command").required();
    add_command.add_argument("-d", "--directory").help("directory").required();
    add_command.add_argument("-r", "--return-code")
        .help("return code")
        .scan<'i', int>()
        .required();

    argparse::ArgumentParser load_command("load");
    load_command.add_description("load history file to database");
    load_command.add_argument("filename").help("history file name");

    argparse::ArgumentParser list_command("list");
    list_command.add_description("list history");
    auto& filter_group = list_command.add_mutually_exclusive_group();
    filter_group.add_argument("-a", "--all")
        .help("list all commands")
        .default_value(false)
        .implicit_value(true);
    filter_group.add_argument("-r", "--recent")
        .help("list recent success commands")
        .default_value(false)
        .implicit_value(true);

    auto& view_group = list_command.add_mutually_exclusive_group();
    view_group.add_argument("-e", "--escape")
        .help("escape special charactors")
        .default_value(false)
        .implicit_value(true);
    view_group.add_argument("--fzf")
        .help("list commands for fzf --read0 --delimiter='\\t' --with-nth=1")
        .default_value(false)
        .implicit_value(true);
    view_group.add_argument("--zsh-history")
        .help("list commands for zsh-history")
        .default_value(false)
        .implicit_value(true);

    program.add_subparser(init_command);
    program.add_subparser(add_command);
    program.add_subparser(load_command);
    program.add_subparser(list_command);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
        std::cerr << program;
        return 1;
    }

    auto config = get_config();

    if (program.is_subcommand_used("init")) {
        return command::init(config.db_path);
    }

    if (program.is_subcommand_used("add")) {
        auto cmd = add_command.get<std::string>("--command");
        auto dir = add_command.get<std::string>("--directory");
        auto ret = add_command.get<int>("--return-code");

        return command::add(config.db_path, cmd, dir, ret);
    }

    if (program.is_subcommand_used("load")) {
        auto filename = load_command.get<std::string>("filename");

        return command::load(config.db_path, filename);
    }

    if (program.is_subcommand_used("list")) {
        auto is_all = list_command.get<bool>("--all");
        auto is_recent = list_command.get<bool>("--recent");

        auto is_escaped = list_command.get<bool>("--escape");
        auto is_fzf = list_command.get<bool>("--fzf");
        auto is_history = list_command.get<bool>("--zsh-history");

        auto filter_mode = is_all      ? FilterMode::All
                           : is_recent ? FilterMode::Recent
                                       : FilterMode::CurrentPath;

        auto view_mode = is_escaped   ? ViewMode::Escaped
                         : is_fzf     ? ViewMode::FZF
                         : is_history ? ViewMode::History
                                      : ViewMode::Normal;

        return command::list(config.db_path, config.recent_num, filter_mode,
                             view_mode);
    }

    std::cerr << program;
    return 1;
}
