/// \file process_queries.hpp
/// \brief Functions for processing queries to `SearchServer`
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_PROCESS_QUERIES_HPP_
#define SEARCH_SERVER_PROCESS_QUERIES_HPP_

#include <search_server/document.hpp>
#include <search_server/export.hpp>
#include <search_server/search_server.hpp>

#include <vector>

namespace search_server {

/// \brief Process `queries` for given `searchServer` and put `std::vector<Document>` for each query
/// into [`std::vector`][1].
///
/// [1]: https://en.cppreference.com/w/cpp/container/vector
///
SEARCH_SERVER_EXPORT
std::vector<std::vector<Document>> processQueries(const SearchServer& searchServer,
                                                  const std::vector<std::string>& queries);

/// \brief Join the results of each query of the corresponding `search_server::processQueries()`
/// call into one [`std::vector`][1].
///
/// [1]: https://en.cppreference.com/w/cpp/container/vector
///
SEARCH_SERVER_EXPORT
std::vector<Document> processQueriesJoined(const SearchServer& searchServer,
                                           const std::vector<std::string>& queries);

} // namespace search_server

#endif // SEARCH_SERVER_PROCESS_QUERIES_HPP_
