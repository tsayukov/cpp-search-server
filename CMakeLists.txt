#[=============================================================================[
  Author: Pavel Tsayukov
  TODO: add license details
  ------------------------------------------------------------------------------
  'search-server' library
  TODO: add brief description
#]=============================================================================]

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(search-server
  VERSION 0.1.0
  DESCRIPTION "TODO: add description"
  HOMEPAGE_URL "https://github.com/tsayukov/cpp-search-server"
  LANGUAGES CXX
)

include(cmake/Common.cmake)
no_in_source_builds_guard(RECURSIVE benchmarks examples include src tests)

use_cxx_standard_at_least(17)
disable_cxx_extensions()

include_project_module(Externals)

add_project_library(search_server)
project_target_link_libraries(search_server
  PUBLIC
    search_server::cxx_standard
)
project_target_compile_options(search_server
  PRIVATE
    ${CXX_OPTIONS}
)
project_target_include_directories(search_server
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)
generate_project_export_header(search_server
  BASE_INCLUDE_DIRECTORY "include"
  INCLUDE_GUARD_NAME "SEARCH_SERVER_EXPORT_HPP_"
  EXPORT_FILE_NAME "search_server/export.hpp"
)

add_subdirectory(src)

include_project_module(DeveloperMode)

if (ENABLE_INSTALL)
  install_project_cmake_configs(COMPATIBILITY "SameMajorVersion")
  install_project_headers()
  install_project_targets()
endif(ENABLE_INSTALL)
