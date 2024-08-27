#include "request_queue.hpp"

RequestQueue::RequestQueue(const SearchServer& search_server) noexcept
        : search_server_(search_server)
        , current_time_(0)
        , no_result_requests_count_(0) {
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    auto result = search_server_.FindTopDocuments(raw_query, status);
    AddRequestResult(result.size());
    return result;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    auto result = search_server_.FindTopDocuments(raw_query);
    AddRequestResult(result.size());
    return result;
}

int RequestQueue::GetNoResultRequests() const noexcept {
    return no_result_requests_count_;
}

void RequestQueue::AddRequestResult(const size_t size) {
    current_time_ += 1;

    requests_.push_back({size, current_time_});
    no_result_requests_count_ += (size == 0) ? 1 : 0;

    while (!requests_.empty() && current_time_ - requests_.front().timestamp >= MIN_IN_DAY_) {
        no_result_requests_count_ -= (requests_.front().count == 0) ? 1 : 0;
        requests_.pop_front();
    }
}
