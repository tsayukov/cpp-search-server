no_in_source_builds_guard()


add_project_library(testing_main INTERFACE)
target_link_libraries(search_server_testing_main
  INTERFACE
    search_server::testing
)
target_sources(search_server_testing_main
  INTERFACE
    main.cpp
)
