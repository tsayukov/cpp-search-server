no_in_source_builds_guard()
cxx_standard_guard()


add_project_library(testing INTERFACE)
target_link_libraries(search_server_testing
  INTERFACE
    search_server_cxx_standard
)
target_include_directories(search_server_testing
  ${WARNING_GUARD}
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
