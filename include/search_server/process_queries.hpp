#ifndef SEARCH_SERVER_PROCESS_QUERIES_HPP_
#define SEARCH_SERVER_PROCESS_QUERIES_HPP_

#include <search_server/document.hpp>
#include <search_server/search_server.hpp>

#include <vector>

namespace search_server {

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
                                                  const std::vector<std::string>& queries);

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server,
                                           const std::vector<std::string>& queries);

} // namespace search_server

#endif // SEARCH_SERVER_PROCESS_QUERIES_HPP_
