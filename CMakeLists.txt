#[=============================================================================[
  Author: Pavel Tsayukov
  TODO: add license details
  ------------------------------------------------------------------------------
  'search-server' library
  TODO: add brief description
#]=============================================================================]

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(search_server
  VERSION 0.1.0
  DESCRIPTION "TODO: add description"
  HOMEPAGE_URL "https://github.com/tsayukov/cpp-search-server"
  LANGUAGES CXX
)

include(cmake/FcmInit.cmake)
fcm_no_in_source_builds_guard(
  RECURSIVE "benchmarks" "examples" "include" "src" "tests"
)

fcm_use_cxx_standard_at_least(17)
fcm_disable_cxx_extensions()

fcm_include("Dependencies" TEMPLATE)

fcm_add_library(search_server)
fcm_target_link_libraries(search_server
  PUBLIC search_server::cxx_standard
)
fcm_target_compile_options(search_server
  PRIVATE ${CXX_OPTIONS}
)
fcm_target_include_directories(search_server
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)
fcm_generate_export_header(search_server
  BASE_INCLUDE_DIRECTORY "include"
  INCLUDE_GUARD_NAME "SEARCH_SERVER_EXPORT_HPP_"
  EXPORT_FILE_NAME "search_server/export.hpp"
)

add_subdirectory(src)
add_subdirectory(extras)

if (ENABLE_TESTING)
  add_subdirectory(tests)
endif()

if (ENABLE_BENCHMARKING)
  add_subdirectory(benchmarks)
endif()

fcm_clang_format_add(format
  DIRECTORIES
    "include"
    "src"
    "extras"
    "tests"
    "benchmarks"
)

fcm_doxygen_add_docs(docs
  FORMAT html
  INPUTS
    "include"
    "README.md"
)

fcm_enable_if(ENABLE_INSTALL)

fcm_install(CONFIGS COMPATIBILITY "SameMajorVersion")
fcm_install(HEADERS)
fcm_install(EXPORT_HEADERS)
fcm_install(TARGETS)
