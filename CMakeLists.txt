cmake_minimum_required(VERSION 3.11)
project("tools" C CXX)

include(FetchContent)

# overwrite install() command with a dummy macro
macro(install)
    # nop
endmacro()

fetchcontent_declare(argparse
                     GIT_REPOSITORY https://github.com/p-ranav/argparse.git)
fetchcontent_makeavailable(argparse)

set(SQLITECPP_INSTALL OFF)
fetchcontent_declare(
    sqlitecpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.3.3)
fetchcontent_makeavailable(sqlitecpp)

fetchcontent_declare(
    fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0)
fetchcontent_makeavailable(fmtlib)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBGIT2 REQUIRED libgit2)
add_library(libgit2 INTERFACE IMPORTED)
set_target_properties(
    libgit2
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LIBGIT2_INCLUDE_DIRS}"
               INTERFACE_LINK_DIRECTORIES "${LIBGIT2_LIBRARY_DIRS}"
               INTERFACE_LINK_LIBRARIES "${LIBGIT2_LIBRARIES}")

add_executable(zhist src/zhist.cpp)
target_compile_features(zhist PRIVATE cxx_std_20)
target_link_libraries(zhist PRIVATE argparse SQLiteCpp sqlite3 pthread dl)

add_executable(
    zprompt src/zprompt.cpp src/zprompt/cwd.cpp src/zprompt/git.cpp
            src/zprompt/ret.cpp src/zprompt/ssh.cpp src/zprompt/venv.cpp)
target_compile_features(zprompt PRIVATE cxx_std_20)
target_include_directories(zprompt PRIVATE include)
target_link_libraries(zprompt PRIVATE libgit2 argparse)

add_executable(zgreeting src/zgreeting.cpp)
target_compile_features(zgreeting PRIVATE cxx_std_20)
target_link_libraries(zgreeting PRIVATE fmt::fmt)

add_executable(tmux-status src/tmux-status.cpp)
target_compile_features(tmux-status PRIVATE cxx_std_20)
target_link_libraries(tmux-status PRIVATE libgit2 argparse)

add_executable(nvim-recent-files src/nvim-recent-files.cpp)
target_compile_features(nvim-recent-files PRIVATE cxx_std_20)

_install(
    TARGETS
    zhist
    zprompt
    zgreeting
    tmux-status
    nvim-recent-files
    RUNTIME
    DESTINATION
    bin)
