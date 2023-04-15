#pragma once

#include "filter.h"
#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <execution>
#include <cmath>
#include <map>
#include <set>
#include <string_view>
#include <string>
#include <tuple>
#include <vector>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
private:
    struct DocumentData {
        std::map<std::string_view, double> word_frequencies;
        int rating;
        DocumentStatus status;
    };

    using Indices = std::map<int, DocumentData>;
    using ReverseIndices = std::map<std::string, std::map<int, double>, std::less<>>;

public:
    // Constructors

    template<typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words);

    // Capacity and Lookup

    [[nodiscard]] int GetDocumentCount() const noexcept;

    [[nodiscard]] const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    // Iterators

    [[nodiscard]] std::set<int>::const_iterator begin() const noexcept;

    [[nodiscard]] std::set<int>::const_iterator end() const noexcept;

    // Modification

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings);

    void RemoveDocument(int document_id);

    // Search

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicate predicate) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string& raw_query,
                                                         DocumentStatus document_status) const;

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    [[nodiscard]] std::tuple<std::vector<std::string>, DocumentStatus>
    MatchDocument(const std::string& raw_query, int document_id) const;

private:
    std::set<std::string, std::less<>> stop_words_;
    std::set<int> document_ids_;
    Indices documents_;
    ReverseIndices word_to_document_frequencies_;

    // Checks

    static void StringHasNotAnyForbiddenChars(std::string_view s);

    static void CheckDocumentIdIsNotNegative(int document_id);

    void CheckDocumentIdDoesntExist(int document_id) const;

    [[nodiscard]] bool IsStopWord(std::string_view word) const;

    // Metric computation

    [[nodiscard]] static int ComputeAverageRating(const std::vector<int>& ratings);

    [[nodiscard]] double ComputeInverseDocumentFrequency(std::size_t number_of_docs_with_the_word) const;

    // Parsing

    [[nodiscard]] std::vector<std::string> SplitIntoWordsNoStop(std::string_view text) const;

    struct QueryWord {
        std::string_view content;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    [[nodiscard]] QueryWord ParseQueryWord(std::string_view word) const;

    [[nodiscard]] Query ParseQuery(std::string_view text) const;

    // Search

    template<typename Predicate>
    [[nodiscard]] std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;
};

// Search Server template implementation

// Constructor

template<typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(
                Filter<std::set<std::string, std::less<>>>(
                        stop_words,
                        [](const auto& word) {
                            return !word.empty() && (StringHasNotAnyForbiddenChars(word), true);
                        })) {
}

// Search

template<typename Predicate>
[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
                                                                   Predicate predicate) const {
    static constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;
    static constexpr double ERROR_MARGIN = 1e-6;

    auto result = FindAllDocuments(ParseQuery(raw_query), predicate);

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
    for (const auto [document_id, relevance] : doc_to_relevance) {
        result.emplace_back(document_id, relevance, documents_.at(document_id).rating);
    }
    return result;
}

// The end of Search Server template implementation