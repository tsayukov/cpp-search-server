no_in_source_builds_guard()


add_project_library(testing INTERFACE)
project_target_link_libraries(testing
  INTERFACE
    search_server::runner
)
project_target_include_directories(testing
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
