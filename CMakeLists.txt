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

include("${PROJECT_SOURCE_DIR}/cmake/Common.cmake")
no_in_source_builds_guard()

use_cxx_standard_at_least(17)
disable_cxx_extensions()

include_project_module(Externals)

add_project_library(search_server)
target_link_libraries(search_server_search_server
  PUBLIC
    search_server_cxx_standard
)
target_include_directories(search_server_search_server
  ${WARNING_GUARD}
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

add_subdirectory(src)

include_project_module(DeveloperMode)

if (ENABLE_INSTALL)
  install_project_cmake_configs(COMPATIBILITY "SameMajorVersion" ARCH_INDEPENDENT)
  # TODO: add other install rules
endif(ENABLE_INSTALL)
