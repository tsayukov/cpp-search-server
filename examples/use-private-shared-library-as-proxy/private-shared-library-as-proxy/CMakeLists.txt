cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(private-shared-library CXX)

find_package(search_server REQUIRED)

if (APPLE)
  set(CMAKE_INSTALL_RPATH "@loader_path")
elseif (UNIX)
  set(CMAKE_INSTALL_RPATH "\$ORIGIN")
endif()

add_library(runner SHARED)
target_link_libraries(runner PUBLIC search_server::search_server)
target_include_directories(runner PUBLIC "../../src")
target_sources(runner PRIVATE "../../src/runner.cpp")
set_target_properties(runner
  PROPERTIES
    SONAME_VERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

include(GNUInstallDirs)
install(TARGETS runner)
install(DIRECTORY "../../src/" TYPE "INCLUDE" FILES_MATCHING PATTERN "*.hpp")
