#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <git2.h>

#include <argparse/argparse.hpp>

constexpr auto color_staged = "#98C379";
constexpr auto color_untrached = "#61AFEF";
constexpr auto color_modified = "#E5C07B";
constexpr auto color_deleted = "#E06C75";
constexpr auto color_conflicted = "#C678DD";

constexpr auto color_ahead = "#98C379";
constexpr auto color_behind = "#E06C75";

class Status {
public:
    void count(git_repository* repo);
    std::string format();

private:
    static std::string color_wrap(const char* color, const std::string& str) {
        return std::format("#[fg={}]{}#[default]", color, str);
    }

    void count_ahead_behind(git_repository* repo);
    void count_status(git_repository* repo);

    size_t ahead_ = 0;
    size_t behind_ = 0;

    int staged_ = 0;
    int untracked_ = 0;
    int modified_ = 0;
    int deleted_ = 0;
    int conflicted_ = 0;
};

constexpr auto flag_untracked = GIT_STATUS_WT_NEW;
constexpr auto flag_modified =
    GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_TYPECHANGE;
constexpr auto flag_staged = GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED |
                             GIT_STATUS_INDEX_TYPECHANGE |
                             GIT_STATUS_INDEX_RENAMED;
constexpr auto flag_deleted = GIT_STATUS_WT_DELETED | GIT_STATUS_INDEX_DELETED;
constexpr auto flag_conflicted = GIT_STATUS_CONFLICTED;

void Status::count_ahead_behind(git_repository* repo) {
    git_reference* head_ref = nullptr;

    if (git_repository_head(&head_ref, repo) == 0) {
        if (git_reference_is_branch(head_ref) == 1) {
            git_reference* upstream_ref = nullptr;

            if (git_branch_upstream(&upstream_ref, head_ref) == 0) {
                const auto* head_oid = git_reference_target(head_ref);
                const auto* upstream_oid = git_reference_target(upstream_ref);

                git_graph_ahead_behind(&ahead_, &behind_, repo, head_oid,
                                       upstream_oid);

                git_reference_free(upstream_ref);
            }
        }

        git_reference_free(head_ref);
    }
}

void Status::count_status(git_repository* repo) {
    git_status_options status_opts = GIT_STATUS_OPTIONS_INIT;
    status_opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    status_opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
                        GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
                        GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;

    git_status_list* status_list = nullptr;

    if (git_status_list_new(&status_list, repo, &status_opts) == 0) {
        auto count = git_status_list_entrycount(status_list);

        for (auto i = 0; i < count; i++) {
            const git_status_entry* entry = git_status_byindex(status_list, i);
            if (entry == nullptr) {
                continue;
            }

            auto status_flags = entry->status;

            if ((status_flags & flag_staged) != 0) {
                staged_++;
            }
            if ((status_flags & flag_untracked) != 0) {
                untracked_++;
            }
            if ((status_flags & flag_modified) != 0) {
                modified_++;
            }
            if ((status_flags & flag_deleted) != 0) {
                deleted_++;
            }
            if ((status_flags & flag_conflicted) != 0) {
                conflicted_++;
            }
        }

        git_status_list_free(status_list);
    }
}

void Status::count(git_repository* repo) {
    count_ahead_behind(repo);
    count_status(repo);
}

std::string Status::format() {
    std::vector<std::string> items;

    if (0 < ahead_) {
        items.emplace_back(color_wrap(color_ahead, std::format("↑{}", ahead_)));
    }
    if (0 < behind_) {
        items.emplace_back(
            color_wrap(color_behind, std::format("↓{}", behind_)));
    }

    if (0 < staged_) {
        items.emplace_back(
            color_wrap(color_staged, std::format("+{}", staged_)));
    }
    if (0 < untracked_) {
        items.emplace_back(
            color_wrap(color_untrached, std::format("?{}", untracked_)));
    }
    if (0 < modified_) {
        items.emplace_back(
            color_wrap(color_modified, std::format("*{}", modified_)));
    }
    if (0 < deleted_) {
        items.emplace_back(
            color_wrap(color_deleted, std::format("x{}", deleted_)));
    }
    if (0 < conflicted_) {
        items.emplace_back(
            color_wrap(color_conflicted, std::format("!{}", conflicted_)));
    }

    auto result = std::accumulate(items.begin(), items.end(), std::string(),
                                  [](std::string acc, const std::string& s) {
                                      return std::move(acc) + s + " ";
                                  });

    return result;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("tmux-status");
    program.add_description("tmux pane status");
    program.add_argument("path").help("path to show status");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    auto path = program.get<std::string>("path");

    git_libgit2_init();

    git_repository* repo = nullptr;

    if (git_repository_open_ext(&repo, path.c_str(), 0, nullptr) == 0) {
        Status status;

        status.count(repo);

        auto result = status.format();

        std::cout << result << "\n";

        git_repository_free(repo);
    }

    git_libgit2_shutdown();

    return 0;
}
