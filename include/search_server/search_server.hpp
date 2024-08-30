#ifndef SEARCH_SERVER_SEARCH_SERVER_HPP_
#define SEARCH_SERVER_SEARCH_SERVER_HPP_

#include <search_server/document.hpp>
#include "search_server/details/concurrent_map.hpp"
#include "search_server/details/string_processing.hpp"

#include <algorithm>
#include <cmath>
#include <execution>
#include <map>
#include <set>
#include <string_view>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

namespace search_server {

class SearchServer {
    struct DocumentData {
        std::map<std::string_view, double> word_frequencies;
        int rating;
        DocumentStatus status;
    };

    using Indices = std::map<int, DocumentData>;

    /// Storage for original words represented as \c std::string.
    /// Words in other containers except stop-words only refer to these.
    using ReverseIndices = std::map<std::string, std::map<int, double>, std::less<>>;

    std::set<std::string, std::less<>> stop_words_;
    std::set<int> document_ids_;
    Indices documents_;
    ReverseIndices word_to_document_frequencies_;

private: // Constants

    static constexpr int kMaxResultDocumentCount = 5;
    static constexpr double kRelevanceErrorMargin = 1e-6;

public: // Constructors

    template
            < typename Container
            , std::enable_if_t<details::kContainsStringViewLike<Container>, int> = 0
            >
    explicit SearchServer(Container&& stop_words);

    explicit SearchServer(std::string_view stop_words);

public: // Capacity

    int GetDocumentCount() const noexcept;

public: // Lookup

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

public: // Iterators

    std::set<int>::const_iterator begin() const noexcept;
    std::set<int>::const_iterator end() const noexcept;

public: // Modifiers

    void AddDocument(int document_id,
                     std::string_view document,
                     DocumentStatus status,
                     const std::vector<int>& ratings);

    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);

public: // Search

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                           Predicate predicate) const;

    template <typename ExecutionPolicy, typename Predicate>
    [[nodiscard]]
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy,
                                           std::string_view raw_query,
                                           Predicate predicate) const;

    [[nodiscard]]
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                           DocumentStatus document_status) const;

    template <typename ExecutionPolicy>
    [[nodiscard]]
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy,
                                           std::string_view raw_query,
                                           DocumentStatus document_status) const;

    [[nodiscard]]
    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    template <typename ExecutionPolicy>
    [[nodiscard]]
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy,
                                           std::string_view raw_query) const;

    using MatchingWordsAndDocStatus = std::tuple<std::vector<std::string_view>, DocumentStatus>;

    [[nodiscard]]
    MatchingWordsAndDocStatus MatchDocument(std::string_view raw_query, int document_id) const;

    [[nodiscard]]
    MatchingWordsAndDocStatus MatchDocument(const std::execution::sequenced_policy&,
                                            std::string_view raw_query,
                                            int document_id) const;

    [[nodiscard]]
    MatchingWordsAndDocStatus MatchDocument(const std::execution::parallel_policy&,
                                            std::string_view raw_query,
                                            int document_id) const;

private: // Checks

    static void StringHasNotAnyForbiddenChars(std::string_view s);

    static void CheckDocumentIdIsNotNegative(int document_id);

    void CheckDocumentIdDoesntExist(int document_id) const;

    void CheckDocumentIdExists(int document_id) const;

    bool IsStopWord(std::string_view word) const;

private: // Metric computation

    static int ComputeAverageRating(const std::vector<int>& ratings);

    double ComputeInverseDocumentFrequency(std::size_t docs_with_the_word) const;

private: // Parsing

    [[nodiscard]]
    std::vector<std::string> SplitIntoWordsNoStop(std::string_view text) const;

    struct QueryWord {
        std::string_view content;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    [[nodiscard]]
    QueryWord ParseQueryWord(std::string_view word) const;

    enum class WordsRepeatable : bool { kYes = true, kNo = false, };

    template <typename ExecutionPolicy>
    [[nodiscard]]
    Query ParseQuery(const ExecutionPolicy& policy,
                     std::string_view text,
                     WordsRepeatable words_can_be_repeated) const;

private: // Search

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&,
                                           const Query& query,
                                           Predicate predicate) const;

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> FindAllDocuments(const std::execution::parallel_policy& par_policy,
                                           const Query& query,
                                           Predicate predicate) const;

    template <typename ExecutionPolicy, typename Map, typename Predicate>
    void ComputeDocumentsRelevance(const ExecutionPolicy& policy,
                                   Map& document_to_relevance,
                                   const Query& query,
                                   Predicate predicate) const;

    [[nodiscard]]
    std::vector<Document> PrepareResult(const std::map<int, double>& document_to_relevance) const;
};

void RemoveDuplicates(SearchServer& server, std::vector<int>* removed_ids = nullptr);

} // namespace search_server

#define SEARCH_SERVER_DETAILS_SEARCH_SERVER_HPP_INC_
#include <search_server/details/search_server.hpp.inc>
#undef SEARCH_SERVER_DETAILS_SEARCH_SERVER_HPP_INC_

#endif // SEARCH_SERVER_SEARCH_SERVER_HPP_
