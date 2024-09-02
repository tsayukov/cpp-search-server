no_in_source_builds_guard()
cxx_standard_guard()


add_project_library(benchmarking)
target_link_libraries(search_server_benchmarking
  PUBLIC
    search_server::cxx_standard
)
target_include_directories(search_server_benchmarking
  ${WARNING_GUARD}
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_sources(search_server_benchmarking
  PRIVATE
    log_duration.cpp
)
