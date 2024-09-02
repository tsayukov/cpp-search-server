no_in_source_builds_guard()


add_project_library(benchmarking_main INTERFACE)
target_link_libraries(search_server_benchmarking_main
  INTERFACE
    search_server::benchmarking
    search_server::runner
)
target_sources(search_server_benchmarking_main
  INTERFACE
    main.cpp
)
