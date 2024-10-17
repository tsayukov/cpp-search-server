#include "search_server/process_queries.hpp"

#include <execution>

namespace search_server {

std::vector<std::vector<Document>> processQueries(const SearchServer& searchServer,
                                                  const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> results(queries.size());
    std::transform(
            std::execution::par, queries.begin(), queries.end(), results.begin(),
            [&searchServer](const auto& query) { return searchServer.findTopDocuments(query); });
    return results;
}

std::vector<Document> processQueriesJoined(const SearchServer& searchServer,
                                           const std::vector<std::string>& queries) {
    std::vector<Document> result;
    for (auto& queryResult : processQueries(searchServer, queries)) {
        for (Document doc : queryResult) {
            result.push_back(doc);
        }
    }
    return result;
}

} // namespace search_server
