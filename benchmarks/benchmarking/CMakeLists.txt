no_in_source_builds_guard(RECURSIVE include)


add_project_library(benchmarking EXCLUDE_FROM_INSTALLATION)
project_target_link_libraries(benchmarking
  PUBLIC
    search_server::cxx_standard
)
project_target_compile_options(benchmarking
  PRIVATE
    ${CXX_OPTIONS}
)
project_target_include_directories(benchmarking
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
project_target_sources(benchmarking
  PRIVATE
    log_duration.cpp
)
