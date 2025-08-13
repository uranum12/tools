cmake_minimum_required(VERSION 3.11)
project("tools" C CXX)

include(cmake/get_cpm.cmake)

cpmaddpackage("gh:p-ranav/argparse#v3.2")
cpmaddpackage("gh:SRombauts/SQLiteCpp#3.3.3")
cpmaddpackage("gh:fmtlib/fmt#11.2.0")
cpmaddpackage("gh:marzer/tomlplusplus#v3.4.0")
cpmaddpackage("gh:Neargye/magic_enum#v0.9.7")

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBGIT2 REQUIRED libgit2)
add_library(libgit2 INTERFACE IMPORTED)
set_target_properties(
    libgit2
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LIBGIT2_INCLUDE_DIRS}"
               INTERFACE_LINK_DIRECTORIES "${LIBGIT2_LIBRARY_DIRS}"
               INTERFACE_LINK_LIBRARIES "${LIBGIT2_LIBRARIES}")

add_executable(
    zhist
    src/zhist.cpp
    src/zhist/command/add.cpp
    src/zhist/command/init.cpp
    src/zhist/command/list.cpp
    src/zhist/command/load.cpp
    src/zhist/config.cpp
    src/zhist/sql/insert.cpp
    src/zhist/sql/misc.cpp
    src/zhist/sql/select.cpp)
target_compile_features(zhist PRIVATE cxx_std_20)
target_include_directories(zhist PRIVATE include)
target_link_libraries(zhist PRIVATE argparse::argparse SQLiteCpp sqlite3
                                    pthread dl tomlplusplus::tomlplusplus)

add_executable(
    zprompt
    src/zprompt.cpp
    src/zprompt/config.cpp
    src/zprompt/cwd.cpp
    src/zprompt/git.cpp
    src/zprompt/ret.cpp
    src/zprompt/ssh.cpp
    src/zprompt/venv.cpp)
target_compile_features(zprompt PRIVATE cxx_std_20)
target_include_directories(zprompt PRIVATE include)
target_link_libraries(
    zprompt PRIVATE libgit2 argparse::argparse tomlplusplus::tomlplusplus
                    magic_enum::magic_enum)

add_executable(zgreeting src/zgreeting.cpp)
target_compile_features(zgreeting PRIVATE cxx_std_20)
target_link_libraries(zgreeting PRIVATE fmt::fmt)

add_executable(tmux-status src/tmux-status.cpp)
target_compile_features(tmux-status PRIVATE cxx_std_20)
target_link_libraries(tmux-status PRIVATE libgit2 argparse)

add_executable(nvim-recent-files src/nvim-recent-files.cpp)
target_compile_features(nvim-recent-files PRIVATE cxx_std_20)

install(TARGETS zhist zprompt zgreeting tmux-status nvim-recent-files
        RUNTIME DESTINATION bin)
