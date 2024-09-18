cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(use-subdirectory CXX)

add_subdirectory(search_server)

add_library(runner)
target_link_libraries(runner PUBLIC search_server::search_server)
target_include_directories(runner PUBLIC "../src")
target_sources(runner PRIVATE "../src/runner.cpp")

add_executable(main)
target_link_libraries(main PRIVATE runner)
target_sources(main PRIVATE "../src/main.cpp")
