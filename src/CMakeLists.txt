no_in_source_builds_guard()


if (ENABLE_TESTING OR ENABLE_BENCHMARKING)
  add_subdirectory(runner)
endif()

add_subdirectory(search-server)
