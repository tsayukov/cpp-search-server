no_in_source_builds_guard()


add_project_executable(testing_main)
target_link_libraries(search_server_testing_main
  PUBLIC
    search_server_testing
)
target_sources(search_server_testing_main
  INTERFACE
    main.cpp
)
