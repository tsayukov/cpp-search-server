no_in_source_builds_guard()


add_project_library(benchmarking_main INTERFACE EXCLUDE_FROM_INSTALLATION)
project_target_link_libraries(benchmarking_main
  INTERFACE
    search_server::benchmarking
    search_server::runner
)
project_target_sources(benchmarking_main
  INTERFACE
    main.cpp
)
