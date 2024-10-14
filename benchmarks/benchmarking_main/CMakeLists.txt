fcm_add_library(benchmarking_main INTERFACE EXCLUDE_FROM_INSTALL)
fcm_target_link_libraries(benchmarking_main
  INTERFACE
    search_server::benchmarking
    search_server::runner
)
fcm_target_sources(benchmarking_main
  INTERFACE main.cpp
)
