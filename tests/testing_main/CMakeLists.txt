fcm_add_library(testing_main INTERFACE EXCLUDE_FROM_INSTALL)
fcm_target_link_libraries(testing_main
  INTERFACE search_server::testing
)
fcm_target_sources(testing_main
  INTERFACE main.cpp
)
