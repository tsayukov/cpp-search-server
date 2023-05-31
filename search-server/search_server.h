#pragma once

#include "document.h"
#include "string_processing.h"
#include "concurrent_map.h"

#include <algorithm>
#include <execution>
#include <cmath>
#include <map>
#include <set>
#include <string_view>
#include <string>
#include <tuple>
#include <vector>
#include <thread>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
private:
    inline static constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;
    inline static constexpr double ERROR_MARGIN = 1e-6;

    struct DocumentData {
        std::map<std::string_view, double> word_frequencies;
        int rating;
        DocumentStatus status;
    };

    using Indices = std::map<int, DocumentData>;
    using ReverseIndices = std::map<std::string, std::map<int, double>, std::less<>>;

public:
    // Constructors

    template<typename StringContainer,
             std::enable_if_t<
                    std::is_same_v<typename std::remove_reference_t<StringContainer>::value_type, std::string>,
                    bool> = true>
    explicit SearchServer(StringContainer&& stop_words);

    template<typename StringViewContainer,
             std::enable_if_t<
                    std::is_same_v<typename std::remove_reference_t<StringViewContainer>::value_type, std::string_view>,
                    bool> = true>
    explicit SearchServer(StringViewContainer&& stop_words);

    explicit SearchServer(std::string_view stop_words);

    // Capacity and Lookup

    [[nodiscard]] int GetDocumentCount() const noexcept;

    [[nodiscard]] const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    // Iterators

    [[nodiscard]] std::set<int>::const_iterator begin() const noexcept;

    [[nodiscard]] std::set<int>::const_iterator end() const noexcept;

    // Modification

    void AddDocument(int document_id, std::string_view document, DocumentStatus status,
                     const std::vector<int>& ratings);

    void RemoveDocument(int document_id);

    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);

    void RemoveDocument(const std::execution::parallel_policy&, int document_id);

    // Search

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query, Predicate predicate) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::execution::sequenced_policy&,
                                                         std::string_view raw_query, Predicate predicate) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::execution::parallel_policy& par_policy,
                                                         std::string_view raw_query, Predicate predicate) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                                         DocumentStatus document_status) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::execution::sequenced_policy&,
                                                         std::string_view raw_query,
                                                         DocumentStatus document_status) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::execution::parallel_policy&,
                                                         std::string_view raw_query,
                                                         DocumentStatus document_status) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::execution::sequenced_policy&,
                                                         std::string_view raw_query) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::execution::parallel_policy& policy,
                                                         std::string_view raw_query) const;

    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(std::string_view raw_query, int document_id) const;

    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const;

    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus>
    MatchDocument(const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const;

private:
    std::set<std::string, std::less<>> stop_words_;
    std::set<int> document_ids_;
    Indices documents_;
    ReverseIndices word_to_document_frequencies_;

    // Checks

    static void StringHasNotAnyForbiddenChars(std::string_view s);

    static void CheckDocumentIdIsNotNegative(int document_id);

    void CheckDocumentIdDoesntExist(int document_id) const;

    void CheckDocumentIdExists(int document_id) const;

    [[nodiscard]] bool IsStopWord(std::string_view word) const;

    // Metric computation

    [[nodiscard]] static int ComputeAverageRating(const std::vector<int>& ratings);

    [[nodiscard]] double ComputeInverseDocumentFrequency(std::size_t docs_with_the_word) const;

    // Parsing

    [[nodiscard]] std::vector<std::string> SplitIntoWordsNoStop(std::string_view text) const;

    struct QueryWord {
        std::string_view content;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    [[nodiscard]] QueryWord ParseQueryWord(std::string_view word) const;

    [[nodiscard]] Query NonUniqueParseQuery(std::string_view text) const;

    [[nodiscard]] Query UniqueParseQuery(std::string_view text) const;

    template<typename ExecutionPolicy>
    [[nodiscard]] Query UniqueParseQuery(const ExecutionPolicy& policy, std::string_view text) const;

    // Search

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&,
                                                         const Query& query, Predicate predicate) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const std::execution::parallel_policy& par_policy,
                                                         const Query& query, Predicate predicate) const;
};

// Search Server template implementation

// Constructors

template<typename StringContainer,
         std::enable_if_t<
                 std::is_same_v<typename std::remove_reference_t<StringContainer>::value_type, std::string>,
                 bool>>
SearchServer::SearchServer(StringContainer&& stop_words) {
    for (auto& stop_word : stop_words) {
        if (!stop_word.empty() && (StringHasNotAnyForbiddenChars(stop_word), true)) {
            stop_words_.insert(std::move(stop_word));
        }
    }
}

template<typename StringViewContainer,
         std::enable_if_t<
                 std::is_same_v<typename std::remove_reference_t<StringViewContainer>::value_type, std::string_view>,
                 bool>>
SearchServer::SearchServer(StringViewContainer&& stop_words) {
    for (auto stop_word : stop_words) {
        if (!stop_word.empty() && (StringHasNotAnyForbiddenChars(stop_word), true)) {
            stop_words_.insert(std::string(stop_word));
        }
    }
}

// Parsing

template<typename ExecutionPolicy>
[[nodiscard]] SearchServer::Query SearchServer::UniqueParseQuery(const ExecutionPolicy& policy,
                                                                 std::string_view text) const {
    Query query = NonUniqueParseQuery(text);

    std::sort(policy, query.plus_words.begin(), query.plus_words.end());
    auto begin_of_plus_words_to_remove = std::unique(query.plus_words.begin(), query.plus_words.end());
    query.plus_words.erase(begin_of_plus_words_to_remove, query.plus_words.end());

    std::sort(policy, query.minus_words.begin(), query.minus_words.end());
    auto begin_of_minus_words_to_remove = std::unique(query.minus_words.begin(), query.minus_words.end());
    query.minus_words.erase(begin_of_minus_words_to_remove, query.minus_words.end());

    return query;
}

// Search

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                                   Predicate predicate) const {
    auto result = FindAllDocuments(UniqueParseQuery(raw_query), predicate);

    std::sort(result.begin(), result.end(),
              [](const Document& lhs, const Document& rhs) {
                  if (std::abs(lhs.relevance - rhs.relevance) < ERROR_MARGIN) {
                      return lhs.rating > rhs.rating;
                  }
                  return lhs.relevance > rhs.relevance;
              });

    if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
        result.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return result;
}

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(
        const std::execution::sequenced_policy&, std::string_view raw_query, Predicate predicate) const {
    return FindTopDocuments(raw_query, predicate);
}

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(
        const std::execution::parallel_policy& par_policy, std::string_view raw_query, Predicate predicate) const {
    auto result = FindAllDocuments(par_policy, UniqueParseQuery(par_policy, raw_query), predicate);

    std::sort(par_policy,
              result.begin(), result.end(),
              [](const Document& lhs, const Document& rhs) {
                  if (std::abs(lhs.relevance - rhs.relevance) < ERROR_MARGIN) {
                      return lhs.rating > rhs.rating;
                  }
                  return lhs.relevance > rhs.relevance;
              });

    if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
        result.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return result;
}

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Predicate predicate) const {
    std::map<int, double> doc_to_relevance;
    for (const auto& plus_word : query.plus_words) {
        auto iter = word_to_document_frequencies_.find(plus_word);
        if (iter == word_to_document_frequencies_.end()) {
            continue;
        }
        const auto& document_frequencies = iter->second;

        // Computation TF-IDF (term frequency–inverse document frequency)
        // source: https://en.wikipedia.org/wiki/Tf%E2%80%93idf
        const double idf = ComputeInverseDocumentFrequency(document_frequencies.size());
        for (const auto& [document_id, tf] : document_frequencies) {
            const auto& document_data = documents_.at(document_id);
            if (predicate(document_id, document_data.status, document_data.rating)) {
                doc_to_relevance[document_id] += tf * idf;
            }
        }
    }

    for (const auto& minus_word : query.minus_words) {
        auto iter = word_to_document_frequencies_.find(minus_word);
        if (iter == word_to_document_frequencies_.end()) {
            continue;
        }
        const auto& document_frequencies = iter->second;
        for (const auto [document_id, _] : document_frequencies) {
            doc_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> result;
    result.reserve(doc_to_relevance.size());
    for (const auto [document_id, relevance] : doc_to_relevance) {
        result.emplace_back(document_id, relevance, documents_.at(document_id).rating);
    }
    return result;
}

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindAllDocuments(
        const std::execution::sequenced_policy&, const Query& query, Predicate predicate) const {
    return FindAllDocuments(query, predicate);
}

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindAllDocuments(
        const std::execution::parallel_policy& par_policy, const Query& query, Predicate predicate) const {
    ConcurrentMap<int, double> concurrent_doc_to_relevance(std::thread::hardware_concurrency());

    std::for_each(
            par_policy,
            query.plus_words.begin(), query.plus_words.end(),
            [this, predicate, &concurrent_doc_to_relevance](auto plus_word_view) {
                auto iter = word_to_document_frequencies_.find(plus_word_view);
                if (iter == word_to_document_frequencies_.end()) {
                    return;
                }
                const auto& document_frequencies = iter->second;

                // Computation TF-IDF (term frequency–inverse document frequency)
                // source: https://en.wikipedia.org/wiki/Tf%E2%80%93idf
                const double idf = ComputeInverseDocumentFrequency(document_frequencies.size());
                for (const auto& [document_id, tf] : document_frequencies) {
                    const auto& document_data = documents_.at(document_id);
                    if (predicate(document_id, document_data.status, document_data.rating)) {
                        concurrent_doc_to_relevance[document_id].ref_to_value += tf * idf;
                    }
                }
            });

    std::for_each(
            par_policy,
            query.minus_words.begin(), query.minus_words.end(),
            [this, &concurrent_doc_to_relevance](auto minus_word_view) {
                auto iter = word_to_document_frequencies_.find(minus_word_view);
                if (iter == word_to_document_frequencies_.end()) {
                    return;
                }
                const auto& document_frequencies = iter->second;
                for (const auto [document_id, _] : document_frequencies) {
                    concurrent_doc_to_relevance.erase(document_id);
                }
            });

    std::vector<Document> result;
    const auto doc_to_relevance = concurrent_doc_to_relevance.BuildOrdinaryMap();
    result.reserve(doc_to_relevance.size());
    for (const auto [document_id, relevance] : doc_to_relevance) {
        result.emplace_back(document_id, relevance, documents_.at(document_id).rating);
    }
    return result;
}

// The end of Search Server template implementation