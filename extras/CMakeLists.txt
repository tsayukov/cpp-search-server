fcm_get(ENABLE_TESTING)
fcm_get(ENABLE_BENCHMARKING)

if (ENABLE_TESTING OR ENABLE_BENCHMARKING)
  add_subdirectory(runner)
endif()

if (ENABLE_TESTING)
  add_subdirectory(testing)
  add_subdirectory(testing_main)
endif()

if (ENABLE_BENCHMARKING)
  add_subdirectory(benchmarking)
  add_subdirectory(benchmarking_main)
endif()
