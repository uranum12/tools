#include "zprompt.hpp"

#include <optional>
#include <string>
#include <vector>

#include <git2.h>

namespace {

constexpr auto color_git = Color::green;

std::optional<std::string> get_branch_name(git_reference* head_ref) {
    if (git_reference_is_branch(head_ref) == 1) {
        const char* name = nullptr;
        if (git_branch_name(&name, head_ref) == 0 && name != nullptr) {
            return std::string(name);
        }
    }
    return std::nullopt;
}

std::vector<std::string> get_tags(git_repository* repo,
                                  const git_oid* head_oid) {
    std::vector<std::string> tags;
    git_reference_iterator* iter = nullptr;
    git_reference* ref = nullptr;
    const git_oid* tag_target_oid = nullptr;

    if (git_reference_iterator_glob_new(&iter, repo, "refs/tags/*") != 0) {
        return {};
    }

    while (git_reference_next(&ref, iter) != GIT_ITEROVER) {
        git_object* tag_peel = nullptr;

        if (git_reference_peel(&tag_peel, ref, GIT_OBJECT_COMMIT) == 0) {
            tag_target_oid = git_object_id(tag_peel);

            if (git_oid_cmp(head_oid, tag_target_oid) == 0) {
                tags.emplace_back(git_reference_shorthand(ref));
            }

            git_object_free(tag_peel);
        }

        git_reference_free(ref);
    }

    git_reference_iterator_free(iter);

    return tags;
}

std::string get_commit_hash(const git_oid* head_oid) {
    constexpr auto short_oid_len = 9;
    auto commit_hash = std::string(git_oid_tostr_s(head_oid), short_oid_len);
    return commit_hash;
}

}  // namespace

std::string get_git_status() {
    std::string result;

    git_repository* repo = nullptr;
    git_reference* head_ref = nullptr;

    git_libgit2_init();

    if (git_repository_open_ext(&repo, ".", 0, nullptr) == 0) {
        if (git_repository_head(&head_ref, repo) == 0) {
            auto branch_name = get_branch_name(head_ref);
            if (branch_name.has_value()) {
                result = color_wrap(color_git, " " + branch_name.value());
            } else {
                const auto* head_oid = git_reference_target(head_ref);

                if (auto tags = get_tags(repo, head_oid); !tags.empty()) {
                    for (auto& tag : tags) {
                        result += " #" + color_wrap(color_git, tag);
                    }
                } else {
                    auto commit_hash = get_commit_hash(head_oid);
                    result = " @" + color_wrap(color_git, commit_hash);
                }
            }
            git_reference_free(head_ref);
        }
        git_repository_free(repo);
    }
    git_libgit2_shutdown();

    return result;
}
