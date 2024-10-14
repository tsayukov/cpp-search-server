fcm_add_library(runner INTERFACE EXCLUDE_FROM_INSTALL)
fcm_target_link_libraries(runner
  INTERFACE search_server::cxx_standard
)
fcm_target_include_directories(runner
  INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
