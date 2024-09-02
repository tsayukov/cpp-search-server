no_in_source_builds_guard()
cxx_standard_guard()


add_project_library(runner INTERFACE)
target_link_libraries(search_server_runner
  INTERFACE
    search_server::cxx_standard
)
target_include_directories(search_server_runner
  ${WARNING_GUARD}
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
