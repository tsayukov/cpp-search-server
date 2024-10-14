add_subdirectory(benchmarking)
add_subdirectory(benchmarking_main)

fcm_add_executable(benchmarks EXCLUDE_FROM_INSTALL)
fcm_target_compile_options(benchmarks
  PRIVATE ${CXX_OPTIONS}
)
fcm_target_link_libraries(benchmarks
  PRIVATE
    search_server::search_server
    search_server::benchmarking_main
)
fcm_target_sources(benchmarks
  PRIVATE search_server_benchmarks.cpp
)
