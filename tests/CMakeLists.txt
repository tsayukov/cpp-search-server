no_in_source_builds_guard()


add_subdirectory(testing)
add_subdirectory(testing_main)

add_project_executable(search_server_tests)
target_sources(search_server_search_server_tests
  PRIVATE
    search_server_tests.cpp
)
target_link_libraries(search_server_search_server_tests
  PRIVATE
    search_server::search_server
    search_server::testing_main
)
add_test(NAME search_server_tests
  COMMAND search_server_search_server_tests --parallel
)
