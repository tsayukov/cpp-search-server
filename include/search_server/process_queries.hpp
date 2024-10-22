/// \file process_queries.hpp
/// \brief Functions for processing queries to \c search_server::SearchServer

#ifndef SEARCH_SERVER_PROCESS_QUERIES_HPP_
#define SEARCH_SERVER_PROCESS_QUERIES_HPP_

#include <search_server/document.hpp>
#include <search_server/export.hpp>
#include <search_server/search_server.hpp>

#include <vector>

namespace search_server {

/// Process \c queries for given \c searchServer and put \c std::vector<Document>
/// for each query into \c std::vector.
SEARCH_SERVER_EXPORT
std::vector<std::vector<Document>> processQueries(const SearchServer& searchServer,
                                                  const std::vector<std::string>& queries);

/// Join the results of each query of the corresponding \c search_server::processQueries() call
/// into one \c std::vector.
SEARCH_SERVER_EXPORT
std::vector<Document> processQueriesJoined(const SearchServer& searchServer,
                                           const std::vector<std::string>& queries);

} // namespace search_server

#endif // SEARCH_SERVER_PROCESS_QUERIES_HPP_
