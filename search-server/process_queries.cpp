#include "process_queries.h"

#include <execution>

std::vector<std::vector<Document>> ProcessQueries(
        const SearchServer& search_server,
        const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> results(queries.size());
    std::transform(
            std::execution::par,
            queries.begin(), queries.end(),
            results.begin(),
            [&search_server](const auto& query) {
                return search_server.FindTopDocuments(query);
            });
    return results;
}

auto ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries)
        -> decltype(MakeFlattenContainer(ProcessQueries(search_server, queries))) {
    return MakeFlattenContainer(ProcessQueries(search_server, queries));
}