no_in_source_builds_guard()


add_subdirectory(benchmarking)
add_subdirectory(benchmarking_main)

add_project_executable(search_server_benchmarks)
target_link_libraries(search_server_search_server_benchmarks
  PRIVATE
    search_server::search_server
    search_server::benchmarking_main
)
target_sources(search_server_search_server_benchmarks
  PRIVATE
    search_server_benchmarks.cpp
)
