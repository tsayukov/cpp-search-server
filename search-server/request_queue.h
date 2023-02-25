#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) noexcept;

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const noexcept;

private:
    struct QueryResult {
        std::size_t count;
        std::uint64_t timestamp;
    };

    const SearchServer& search_server_;

    std::uint64_t current_time_;
    int no_result_requests_count_;
    std::deque<QueryResult> requests_;

    static constexpr int MIN_IN_DAY_ = 1440;

    void AddRequestResult(const size_t size);
};

// RequestQueue template implementation

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
    AddRequestResult(result.size());
    return result;
}

// The end of RequestQueue template implementation