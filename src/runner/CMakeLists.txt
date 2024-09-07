no_in_source_builds_guard(RECURSIVE include)


add_project_library(runner INTERFACE EXCLUDE_FROM_INSTALLATION)
project_target_link_libraries(runner
  INTERFACE
    search_server::cxx_standard
)
project_target_include_directories(runner
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
