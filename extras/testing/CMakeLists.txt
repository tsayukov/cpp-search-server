fcm_add_library(testing INTERFACE EXCLUDE_FROM_INSTALL)
fcm_target_link_libraries(testing
  INTERFACE search_server::runner
)
fcm_target_include_directories(testing
  INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
