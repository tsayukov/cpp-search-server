no_in_source_builds_guard()
cxx_standard_guard()


add_project_library(testing)
target_link_libraries(search_server_testing
  PUBLIC
    search_server_cxx_standard
)
target_include_directories(search_server_search_server
  ${WARNING_GUARD}
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

add_subdirectory(src)
