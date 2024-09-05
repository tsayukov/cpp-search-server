no_in_source_builds_guard()


add_subdirectory(benchmarking)
add_subdirectory(benchmarking_main)

add_project_executable(search_server_benchmarks EXCLUDE_FROM_INSTALLATION)
project_target_compile_options(search_server_benchmarks
  PRIVATE
    ${CXX_OPTIONS}
)
project_target_link_libraries(search_server_benchmarks
  PRIVATE
    search_server::search_server
    search_server::benchmarking_main
)
project_target_sources(search_server_benchmarks
  PRIVATE
    search_server_benchmarks.cpp
)
