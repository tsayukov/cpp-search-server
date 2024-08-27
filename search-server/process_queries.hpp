#ifndef SEARCH_SERVER_PROCESS_QUERIES_HPP
#define SEARCH_SERVER_PROCESS_QUERIES_HPP

#include <search_server/document.hpp>
#include <search_server/search_server.hpp>

#include "flatten_container.hpp"

#include <vector>

std::vector<std::vector<Document>> ProcessQueries(
        const SearchServer& search_server,
        const std::vector<std::string>& queries);

auto ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries)
        -> decltype(MakeFlattenContainer(ProcessQueries(search_server, queries)));

#endif // SEARCH_SERVER_PROCESS_QUERIES_HPP
