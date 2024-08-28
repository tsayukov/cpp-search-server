#include "search_server/process_queries.hpp"

#include <execution>

namespace search_server {

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
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

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server,
                                           const std::vector<std::string>& queries) {
    std::vector<Document> result;
    for (auto& query_result : ProcessQueries(search_server, queries)) {
        for (Document doc : query_result) {
            result.push_back(doc);
        }
    }
    return result;
}

} // namespace search_server
