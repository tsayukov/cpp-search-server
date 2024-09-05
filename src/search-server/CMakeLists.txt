no_in_source_builds_guard()


project_target_sources(search_server
  PRIVATE
    document.cpp
    process_queries.cpp
    search_server.cpp
    string_processing.cpp
)
