fcm_add_library(benchmarking EXCLUDE_FROM_INSTALL)
fcm_target_link_libraries(benchmarking
  PUBLIC search_server::cxx_standard
)
fcm_target_compile_options(benchmarking
  PRIVATE ${CXX_OPTIONS}
)
fcm_target_include_directories(benchmarking
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
fcm_target_sources(benchmarking
  PRIVATE log_duration.cpp
)
