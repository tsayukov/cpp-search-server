#ifndef SEARCH_SERVER_SEARCH_SERVER_HPP_
#define SEARCH_SERVER_SEARCH_SERVER_HPP_

#include "search_server/details/concurrent_map.hpp"
#include "search_server/details/string_processing.hpp"

#include <search_server/document.hpp>
#include <search_server/export.hpp>

#include <algorithm>
#include <cmath>
#include <execution>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

namespace search_server {

class SEARCH_SERVER_EXPORT SearchServer {
    struct DocumentData {
        std::map<std::string_view, double> wordFrequencies;
        int rating;
        DocumentStatus status;
    };

    using Indices = std::map<int, DocumentData>;

    /// Storage for original words represented as \c std::string.
    /// Words in other containers except stop-words only refer to these.
    using ReverseIndices = std::map<std::string, std::map<int, double>, std::less<>>;

    std::set<std::string, std::less<>> mStopWords;
    std::set<int> mDocumentIds;
    Indices mDocuments;
    ReverseIndices mWordToDocumentFrequencies;

private: // Constants

    static constexpr int kMaxResultDocumentCount = 5;
    static constexpr double kRelevanceErrorMargin = 1e-6;

public: // Constructors

    // clang-format off
    template
            < typename Container
            , std::enable_if_t<details::kContainsStringViewLike<Container>, int> = 0
            >
    // clang-format on
    explicit SearchServer(Container&& stopWords);

    explicit SearchServer(std::string_view stopWords);

public: // Capacity

    int getDocumentCount() const noexcept;

public: // Lookup

    const std::map<std::string_view, double>& getWordFrequencies(int documentId) const;

public: // Iterators

    std::set<int>::const_iterator begin() const noexcept;
    std::set<int>::const_iterator end() const noexcept;

public: // Modifiers

    void addDocument(int documentId,
                     std::string_view document,
                     DocumentStatus status,
                     const std::vector<int>& ratings);

    void removeDocument(int documentId);
    void removeDocument(const std::execution::sequenced_policy&, int documentId);
    void removeDocument(const std::execution::parallel_policy&, int documentId);

public: // Search

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> findTopDocuments(std::string_view rawQuery, Predicate predicate) const;

    template <typename ExecutionPolicy, typename Predicate>
    [[nodiscard]]
    std::vector<Document> findTopDocuments(const ExecutionPolicy& policy,
                                           std::string_view rawQuery,
                                           Predicate predicate) const;

    [[nodiscard]]
    std::vector<Document> findTopDocuments(std::string_view rawQuery,
                                           DocumentStatus documentStatus) const;

    template <typename ExecutionPolicy>
    [[nodiscard]]
    std::vector<Document> findTopDocuments(const ExecutionPolicy& policy,
                                           std::string_view rawQuery,
                                           DocumentStatus documentStatus) const;

    [[nodiscard]]
    std::vector<Document> findTopDocuments(std::string_view rawQuery) const;

    template <typename ExecutionPolicy>
    [[nodiscard]]
    std::vector<Document> findTopDocuments(const ExecutionPolicy& policy,
                                           std::string_view rawQuery) const;

    using MatchingWordsAndDocStatus = std::tuple<std::vector<std::string_view>, DocumentStatus>;

    [[nodiscard]]
    MatchingWordsAndDocStatus matchDocument(std::string_view rawQuery, int documentId) const;

    [[nodiscard]]
    MatchingWordsAndDocStatus matchDocument(const std::execution::sequenced_policy&,
                                            std::string_view rawQuery,
                                            int documentId) const;

    [[nodiscard]]
    MatchingWordsAndDocStatus matchDocument(const std::execution::parallel_policy&,
                                            std::string_view rawQuery,
                                            int documentId) const;

private: // Checks

    static void stringHasNotAnyForbiddenChars(std::string_view s);

    static void checkDocumentIdIsNotNegative(int documentId);

    void checkDocumentIdDoesntExist(int documentId) const;

    void checkDocumentIdExists(int documentId) const;

    bool isStopWord(std::string_view word) const;

private: // Metric computation

    static int computeAverageRating(const std::vector<int>& ratings);

    double computeInverseDocumentFrequency(std::size_t docsWithWord) const;

private: // Parsing

    [[nodiscard]]
    std::vector<std::string> splitIntoWordsNoStop(std::string_view text) const;

    struct QueryWord {
        std::string_view content;
        bool isMinus;
        bool isStop;
    };

    struct Query {
        std::vector<std::string_view> plusWords;
        std::vector<std::string_view> minusWords;
    };

    [[nodiscard]]
    QueryWord parseQueryWord(std::string_view word) const;

    enum class WordsRepeatable : bool {
        kYes = true,
        kNo = false,
    };

    template <typename ExecutionPolicy>
    [[nodiscard]]
    Query parseQuery(const ExecutionPolicy& policy,
                     std::string_view text,
                     WordsRepeatable wordsCanBeRepeated) const;

private: // Search

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> findAllDocuments(const Query& query, Predicate predicate) const;

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> findAllDocuments(const std::execution::sequenced_policy&,
                                           const Query& query,
                                           Predicate predicate) const;

    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document> findAllDocuments(const std::execution::parallel_policy& par_policy,
                                           const Query& query,
                                           Predicate predicate) const;

    template <typename ExecutionPolicy, typename Map, typename Predicate>
    void computeDocumentsRelevance(const ExecutionPolicy& policy,
                                   Map& documentToRelevance,
                                   const Query& query,
                                   Predicate predicate) const;

    [[nodiscard]]
    std::vector<Document> prepareResult(const std::map<int, double>& documentToRelevance) const;
};

SEARCH_SERVER_EXPORT
void removeDuplicates(
        SearchServer& server,
        std::optional<std::reference_wrapper<std::vector<int>>> removedIds = std::nullopt);

} // namespace search_server

#define SEARCH_SERVER_DETAILS_SEARCH_SERVER_INL_HPP_
#include <search_server/details/search_server-inl.hpp>
#undef SEARCH_SERVER_DETAILS_SEARCH_SERVER_INL_HPP_

#endif // SEARCH_SERVER_SEARCH_SERVER_HPP_
