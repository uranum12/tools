cmake_minimum_required(VERSION 3.11)
project("tools" C CXX)

add_library(argparse INTERFACE)
target_include_directories(argparse INTERFACE lib/argparse/include)

add_library(fmt INTERFACE)
target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)
target_include_directories(fmt INTERFACE lib/fmt/include)

add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus INTERFACE lib/tomlplusplus/include)

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE lib/magic_enum/include)

find_package(SQLite3 REQUIRED)
find_package(exiv2 CONFIG NAMES exiv2)

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
target_link_libraries(zhist PRIVATE argparse SQLite::SQLite3 tomlplusplus)

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
target_link_libraries(zprompt PRIVATE libgit2 argparse tomlplusplus magic_enum)

add_executable(zgreeting src/zgreeting.cpp)
target_compile_features(zgreeting PRIVATE cxx_std_20)
target_link_libraries(zgreeting PRIVATE fmt)

add_executable(tmux-status src/tmux-status.cpp)
target_compile_features(tmux-status PRIVATE cxx_std_20)
target_link_libraries(tmux-status PRIVATE libgit2 argparse)

add_executable(nvim-recent-files src/nvim-recent-files.cpp)
target_compile_features(nvim-recent-files PRIVATE cxx_std_20)

add_executable(clean_ds src/clean_ds.cpp)
target_compile_features(clean_ds PRIVATE cxx_std_20)
target_link_libraries(clean_ds PRIVATE argparse)

if(exiv2_FOUND)
    add_executable(imgsort src/imgsort.cpp)
    target_compile_features(imgsort PRIVATE cxx_std_20)
    target_link_libraries(imgsort PRIVATE argparse fmt Exiv2::exiv2lib)
endif()

install(
    TARGETS zhist
            zprompt
            zgreeting
            tmux-status
            nvim-recent-files
            clean_ds
            imgsort
    RUNTIME DESTINATION bin)
