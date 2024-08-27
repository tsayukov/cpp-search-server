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
    // Storage for original words represented as std::string.
    // Words in other containers except stop-words only refer to these.
    using ReverseIndices = std::map<std::string, std::map<int, double>, std::less<>>;

    using MatchingWordsAndDocStatus = std::tuple<std::vector<std::string_view>, DocumentStatus>;

public:
    // Constructors

    template<typename StringContainer, typename ValueType = typename std::decay_t<StringContainer>::value_type,
             std::enable_if_t<std::is_convertible_v<ValueType, std::string_view>, bool> = true>
    explicit SearchServer(StringContainer&& stop_words);

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

    template<typename ExecutionPolicy, typename Predicate>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy,
                                                         std::string_view raw_query, Predicate predicate) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                                         DocumentStatus document_status) const;

    template<typename ExecutionPolicy>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy,
                                                         std::string_view raw_query,
                                                         DocumentStatus document_status) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    template<typename ExecutionPolicy>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy,
                                                         std::string_view raw_query) const;

    [[nodiscard]] MatchingWordsAndDocStatus MatchDocument(std::string_view raw_query, int document_id) const;

    [[nodiscard]] MatchingWordsAndDocStatus MatchDocument(const std::execution::sequenced_policy&,
                                                          std::string_view raw_query, int document_id) const;

    [[nodiscard]] MatchingWordsAndDocStatus MatchDocument(const std::execution::parallel_policy&,
                                                          std::string_view raw_query, int document_id) const;

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

    enum class WordsRepeatable : bool { Yes = true, No = false, };

    template<typename ExecutionPolicy>
    [[nodiscard]] Query ParseQuery(const ExecutionPolicy& policy,
                                   std::string_view text, WordsRepeatable words_can_be_repeated) const;

    // Search

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&,
                                                         const Query& query, Predicate predicate) const;

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const std::execution::parallel_policy& par_policy,
                                                         const Query& query, Predicate predicate) const;

    template<typename ExecutionPolicy, typename Map, typename Predicate>
    void ComputeDocumentsRelevance(const ExecutionPolicy& policy,
                                   Map& document_to_relevance,
                                   const Query& query, Predicate predicate) const;

    [[nodiscard]] std::vector<Document> PrepareResult(const std::map<int, double>& document_to_relevance) const;
};

// Search Server template implementation

// Constructors

template<typename StringContainer, typename ValueType,
         std::enable_if_t<std::is_convertible_v<ValueType, std::string_view>, bool>>
SearchServer::SearchServer(StringContainer&& stop_words) {
    static_assert(std::is_same_v<typename std::decay_t<StringContainer>::value_type, ValueType>,
                  "ValueType must not be passed by a user; it must be deduced as value_type of StringContainer and "
                  "it exists only to make std::enable_if_t less verbose");

    for (auto& stop_word : stop_words) {
        if (!stop_word.empty() && (StringHasNotAnyForbiddenChars(stop_word), true)) {
            stop_words_.insert(std::string(std::move(stop_word)));
        }
    }
}

// Parsing

template<typename ExecutionPolicy>
[[nodiscard]] SearchServer::Query SearchServer::ParseQuery(
        const ExecutionPolicy& policy, std::string_view text, WordsRepeatable words_can_be_repeated) const {
    Query query;
    const auto words = SplitIntoWordsView(text);
    for (const auto word : words) {
        StringHasNotAnyForbiddenChars(word);
        const auto query_word = ParseQueryWord(word);
        if (!(query_word.is_stop)) {
            if (query_word.is_minus) {
                query.minus_words.push_back(query_word.content);
            } else {
                query.plus_words.push_back(query_word.content);
            }
        }
    }

    if (static_cast<bool>(words_can_be_repeated)) {
        return query;
    }

    RemoveDuplicateWords(policy, query.plus_words);
    RemoveDuplicateWords(policy, query.minus_words);

    return query;
}

// Search

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                                   Predicate predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template<typename ExecutionPolicy, typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(
        const ExecutionPolicy& policy, std::string_view raw_query, Predicate predicate) const {
    auto result = FindAllDocuments(
            policy,
            ParseQuery(policy, raw_query, WordsRepeatable::No),
            predicate);

    std::sort(policy,
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

template<typename ExecutionPolicy>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(
        const ExecutionPolicy& policy, std::string_view raw_query, DocumentStatus document_status) const {
    return FindTopDocuments(
            policy,
            raw_query,
            [document_status](int /*document_id*/, DocumentStatus status, int /*rating*/) {
                return status == document_status;
            });
}

template<typename ExecutionPolicy>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(
        const ExecutionPolicy& policy, std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Predicate predicate) const {
    std::map<int, double> doc_to_relevance;
    ComputeDocumentsRelevance(std::execution::seq, doc_to_relevance, query, predicate);
    return PrepareResult(doc_to_relevance);
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
    ComputeDocumentsRelevance(par_policy, concurrent_doc_to_relevance, query, predicate);
    return PrepareResult(concurrent_doc_to_relevance.BuildOrdinaryMap());
}

template<typename ExecutionPolicy, typename Map, typename Predicate>
void SearchServer::ComputeDocumentsRelevance(const ExecutionPolicy& policy,
                                             Map& document_to_relevance,
                                             const Query& query, Predicate predicate) const {
    static_assert(std::is_integral_v<typename Map::key_type> && std::is_floating_point_v<typename Map::mapped_type>);

    std::for_each(
            policy,
            query.plus_words.begin(), query.plus_words.end(),
            [this, predicate, &document_to_relevance](auto plus_word_view) {
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
                        document_to_relevance[document_id] += tf * idf;
                    }
                }
            });

    std::for_each(
            policy,
            query.minus_words.begin(), query.minus_words.end(),
            [this, &document_to_relevance](auto minus_word_view) {
                auto iter = word_to_document_frequencies_.find(minus_word_view);
                if (iter == word_to_document_frequencies_.end()) {
                    return;
                }
                const auto& document_frequencies = iter->second;
                for (const auto [document_id, _] : document_frequencies) {
                    document_to_relevance.erase(document_id);
                }
            });
}

// The end of Search Server template implementation