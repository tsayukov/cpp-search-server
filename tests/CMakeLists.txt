foreach (suffix IN ITEMS tests parallel_tests)
  fcm_add_executable(${suffix} EXCLUDE_FROM_INSTALL)
  fcm_target_compile_options(${suffix}
    PRIVATE ${CXX_OPTIONS}
  )
  fcm_target_sources(${suffix}
    PRIVATE search_server_${suffix}.cpp
  )
  fcm_target_link_libraries(${suffix}
    PRIVATE
      search_server::search_server
      search_server::testing_main
  )
endforeach()

fcm_valgrind_add_custom_target(FOR tests)
fcm_valgrind_add_custom_target(UNITY memcheck)

add_test(NAME search_server.tests
  COMMAND search_server_tests --parallel
)

add_test(NAME search_server.parallel_tests
  COMMAND search_server_parallel_tests
)
