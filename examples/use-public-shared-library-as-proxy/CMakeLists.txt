cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(use-public-shared-library-as-proxy CXX)

find_path(runner_include runner.hpp REQUIRED)
find_library(runner_library runner REQUIRED)

add_library(runner SHARED IMPORTED)
set_target_properties(runner
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${runner_include}"
    IMPORTED_LOCATION "${runner_library}"
)

find_package(search_server REQUIRED)

add_executable(main)
target_link_libraries(main PRIVATE runner search_server::search_server)
target_sources(main PRIVATE "../src/main.cpp")

include(GNUInstallDirs)
install(TARGETS main)
