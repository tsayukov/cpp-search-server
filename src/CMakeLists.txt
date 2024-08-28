no_in_source_builds_guard()


target_sources(search_server_search_server
  PRIVATE
    src/document.cpp
    src/process_queries.cpp
    src/search_server.cpp
    src/string_processing.cpp
)
