fcm_add_executable(tests EXCLUDE_FROM_INSTALL)
fcm_target_compile_options(tests
  PRIVATE ${CXX_OPTIONS}
)
fcm_target_sources(tests
  PRIVATE search_server_tests.cpp
)
fcm_target_link_libraries(tests
  PRIVATE
    search_server::search_server
    search_server::testing_main
)
fcm_valgrind_add_custom_target(FOR tests)

add_test(NAME search_server.tests
  COMMAND search_server_tests --parallel
)

fcm_valgrind_add_custom_target(UNITY memcheck)
