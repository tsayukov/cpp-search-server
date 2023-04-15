#include "test_example_functions.h"

#include "log_duration.h"

using namespace std::string_literals;

void AddDocument(SearchServer &server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) {
    LOG_DURATION(__FUNCTION__ + " operation time"s);
    server.AddDocument(document_id, document, status, ratings);
}

void RemoveDocument(SearchServer& server, int document_id) {
    LOG_DURATION(__FUNCTION__ + " operation time"s);
    server.RemoveDocument(document_id);
}

std::vector<Document> FindTopDocuments(const SearchServer& server, const std::string& raw_query) {
    LOG_DURATION(__FUNCTION__ + " operation time"s);
    return server.FindTopDocuments(raw_query);
}

std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const SearchServer& server,
                                                                   const std::string& raw_query,
                                                                   int document_id) {
    LOG_DURATION(__FUNCTION__ + " operation time"s);
    return server.MatchDocument(raw_query, document_id);
}

const std::map<std::string_view, double>& GetWordFrequencies(const SearchServer& server, int document_id) {
    LOG_DURATION(__FUNCTION__ + " operation time"s);
    return server.GetWordFrequencies(document_id);
}

void RemoveDuplicatesWithProfiling(SearchServer& server) {
    LOG_DURATION(__FUNCTION__ + " operation time"s);
    RemoveDuplicates(server);
}