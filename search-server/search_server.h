#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "document.h"
#include "string_processing.h"

inline constexpr double ERROR_MARGIN = 1e-6;

inline constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template<typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words);

    int GetDocumentCount() const noexcept;

    int GetDocumentId(int index) const;

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings);

    template<typename Predicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, Predicate predicate) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus document_status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
                                                                       int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> ids_;

    static void StringHasNotAnyForbiddenChars(const std::string& s);

    static void DocumentIdIsNotNegative(int document_id);

    void DocumentIdDoesntExist(int document_id) const;

    [[nodiscard]] bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string content;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    QueryWord ParseQueryWord(std::string word) const;

    Query ParseQuery(const std::string& text) const;

    template<typename Predicate>
    std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;
};

template<typename Receiver, typename Source,
        typename Predicate, typename Inserter>
Receiver Filter(const Source& source,
                Predicate predicate, Inserter inserter,
                Receiver default_ctor = Receiver())
{
    Receiver receiver = default_ctor;
    for (const auto& elem : source) {
        if (predicate(elem)) {
            inserter(receiver, elem);
        }
    }
    return receiver;
}

inline constexpr auto SET_INSERTER = [](auto& receiver, const auto& elem) -> void {
    receiver.insert(elem);
};

// Search Server template implementation

template<typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(Filter<std::set<std::string>>(
                stop_words,
                [](const auto& elem) {
                    if (elem.empty()) {
                        return false;
                    }
                    StringHasNotAnyForbiddenChars(elem);
                    return true;
                },
                SET_INSERTER)) {
}

template<typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, Predicate predicate) const {
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
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Predicate predicate) const {
    std::map<int, double> doc_to_relevance;
    for (const auto& plus_word : query.plus_words) {
        auto iter = word_to_document_freqs_.find(plus_word);
        if (iter != word_to_document_freqs_.end()) {
            const auto& document_freqs = iter->second;
            const double idf = std::log(GetDocumentCount() * 1.0 / document_freqs.size());
            for (const auto& [document_id, tf] : document_freqs) {
                const auto& document_data = documents_.at(document_id);
                if (predicate(document_id, document_data.status, document_data.rating)) {
                    doc_to_relevance[document_id] += tf * idf;
                }
            }
        }
    }

    for (const auto& minus_word : query.minus_words) {
        auto iter = word_to_document_freqs_.find(minus_word);
        if (iter != word_to_document_freqs_.end()) {
            const auto& document_freqs = iter->second;
            for (const auto [document_id, _] : document_freqs) {
                doc_to_relevance.erase(document_id);
            }
        }
    }

    std::vector<Document> result;
    for (const auto [document_id, relevance] : doc_to_relevance) {
        result.emplace_back(document_id, relevance, documents_.at(document_id).rating);
    }
    return result;
}

// The end of Search Server template implementation