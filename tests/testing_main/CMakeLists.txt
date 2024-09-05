no_in_source_builds_guard()


add_project_library(testing_main INTERFACE EXCLUDE_FROM_INSTALLATION)
project_target_link_libraries(testing_main
  INTERFACE
    search_server::testing
)
project_target_sources(testing_main
  INTERFACE
    main.cpp
)
