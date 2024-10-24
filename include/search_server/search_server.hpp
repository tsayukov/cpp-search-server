/// \file search_server.hpp
/// \brief A search server for indexing documents and searching for more relevant documents matching
/// a query
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_SEARCH_SERVER_HPP_
#define SEARCH_SERVER_SEARCH_SERVER_HPP_

#include <search_server/details/string_processing.hpp>
#include <search_server/document.hpp>
#include <search_server/export.hpp>

#include <execution>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace search_server {

/// \brief A search server for indexing documents and searching by query with ranking of results.
///
/// In terms of this class, a \em document is plain text with words separated by spaces,
/// that is, ' '. So far, it doesn't support other whitespace characters as line feed, tabulation,
/// and so forth. Each document that have been indexed is assigned a unique identifier. Words that
/// are <em>stop words</em> will not be indexed. Stop words, if any, must be defined during
/// initialization of SearchServer.
///
/// A \em query is also plain text with words separated by spaces. These words are searched among
/// the indexed documents, except for those that start with a single minus sign `-`, they are also
/// called “minus words”.
///
/// The relevance of documents is calculated using the [TF-IDF][1] method, which may be helpful
/// in the following cases:
/// - highlighting the most important words in text;
/// - grouping texts by similarity.
///
/// The length of a document is not normalized, so long documents may have higher TF, even
/// if keywords are less common, and in such cases, TF-IDF may underestimate the importance
/// of specific words.
///
/// [1]: https://en.wikipedia.org/wiki/Tf%E2%80%93idf
///
class SEARCH_SERVER_EXPORT SearchServer {
    struct DocumentData {
        std::map<std::string_view, double> wordFrequencies;
        int rating;
        DocumentStatus status;
    };

    using Indices = std::unordered_map<int, DocumentData>;

    /// Storage for original words represented as `std::string`.
    /// Words in other containers except stop-words only refer to these.
    using ReverseIndices = std::map<std::string, std::unordered_map<int, double>, std::less<>>;

    std::set<std::string, std::less<>> mStopWords;
    std::set<int> mDocumentIds;
    Indices mDocuments;
    ReverseIndices mWordToDocumentFrequencies;

private: // Constants

    static constexpr double kRelevanceErrorMargin = 1e-6;

public:

    static constexpr std::size_t kMaxResultDocumentCount = 5;

public: // Constructors

    /// \brief Constructs the search server with stop words from the `stopWords` container with
    /// values of a type that can be converted to [`std::string_view`][1].
    ///
    /// \exception std::invalid_argument if stop words contain forbidden characters from `0x00` to
    /// `0x1F`.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/string/basic_string_view
    ///
    // clang-format off
    template
            < typename Container
            , std::enable_if_t<details::kContainsStringViewLike<Container>, int> = 0
            >
    // clang-format on
    explicit SearchServer(Container&& stopWords);

    /// \brief Constructs the search server with stop words separated by at least one whitespace
    /// in `stopWords`.
    ///
    /// \exception std::invalid_argument if stop words contain forbidden characters from `0x00` to
    /// `0x1F`.
    ///
    explicit SearchServer(std::string_view stopWords);

public: // Capacity

    /// \brief Return the number of indexed documents in the search server.
    int getDocumentCount() const noexcept;

public: // Lookup

    /// \brief Return the associative container with key-value pairs of words and their term
    /// frequencies.
    ///
    /// Keys are all non-stop words in the document `documentId` and the values are term frequencies
    /// of the corresponding words, i.e., the number of times that a word occurs in the document
    /// divided by the total number of non-stop words in the document.
    ///
    const std::map<std::string_view, double>& getWordFrequencies(int documentId) const;

public: // Iterators

    /// \brief Return an iterator to the smallest document id.
    std::set<int>::const_iterator begin() const noexcept;

    /// \brief Return an iterator to the element following the largest document id.
    std::set<int>::const_iterator end() const noexcept;

public: // Modifiers

    /// \brief Index `document` as a document with `documentId` with DocumentStatus `status`
    /// and `ratings`.
    ///
    /// \exception std::invalid_argument if `documentId` is negative or already exists.
    /// \exception std::invalid_argument if the document contains forbidden characters from `0x00`
    /// to `0x1F`.
    ///
    /// \sa [std::invalid_argument](https://en.cppreference.com/w/cpp/error/invalid_argument)
    ///
    void addDocument(int documentId,
                     std::string_view document,
                     DocumentStatus status,
                     const std::vector<int>& ratings);

    /// \brief Remove the document with `documentId`.
    void removeDocument(int documentId);

    /// \brief Effectively call `removeDocument(int)`.
    void removeDocument(const std::execution::sequenced_policy&, int documentId);

    /// \brief Remove the document with `documentId`, but execute according
    /// to the [std::execution::par][1] parallel policy.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag
    ///
    void removeDocument(const std::execution::parallel_policy&, int documentId);

public: // Search

    /// \brief Search for at most `topDocumentsCount` relevant documents that match `rawQuery`
    /// and satisfy `predicate`.
    ///
    /// \param rawQuery the query.
    /// It supports for minus words in the `-<word>` format.
    /// \param predicate the predicate which returns `true` for the required document.
    /// \parblock
    /// It must have parameters in the following order:
    /// - `int documentId`,
    /// - `DocumentStatus status`,
    /// - `int rating`.
    /// \endparblock
    /// \param topDocumentsCount the largest number of the returned documents.
    /// By default, it is `kMaxResultDocumentCount`.
    ///
    /// \return [`std::vector`][1] of relevant documents.
    ///
    /// \exception std::invalid_argument if `rawQuery` contains forbidden characters from `0x00` to
    /// `0x1F`.
    /// \exception std::invalid_argument if `rawQuery` contains a word with one character `-`
    /// or a word started with two consecutive `-`.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/container/vector
    ///
    template <typename Predicate>
    [[nodiscard]]
    std::vector<Document>
    findTopDocuments(std::string_view rawQuery,
                     Predicate predicate,
                     std::size_t topDocumentsCount = kMaxResultDocumentCount) const;

    /// \brief Same as `findTopDocuments(std::string_view, Predicate, std::size_t) const`,
    /// but execute according to the [execution][1] `policy`.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/algorithm#Execution_policies
    ///
    template <typename ExecutionPolicy, typename Predicate>
    [[nodiscard]]
    std::vector<Document>
    findTopDocuments(const ExecutionPolicy& policy,
                     std::string_view rawQuery,
                     Predicate predicate,
                     std::size_t topDocumentsCount = kMaxResultDocumentCount) const;

    /// \brief Same as `findTopDocuments(std::string_view, Predicate, std::size_t) const`,
    /// but search for only those documents whose status is `documentStatus`.
    [[nodiscard]]
    std::vector<Document>
    findTopDocuments(std::string_view rawQuery,
                     DocumentStatus documentStatus,
                     std::size_t topDocumentsCount = kMaxResultDocumentCount) const;

    /// \brief Same as `findTopDocuments(std::string_view, documentStatus, std::size_t) const`,
    /// but execute according to the [execution][1] `policy`.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/algorithm#Execution_policies
    ///
    template <typename ExecutionPolicy>
    [[nodiscard]]
    std::vector<Document>
    findTopDocuments(const ExecutionPolicy& policy,
                     std::string_view rawQuery,
                     DocumentStatus documentStatus,
                     std::size_t topDocumentsCount = kMaxResultDocumentCount) const;

    /// \brief Same as `findTopDocuments(std::string_view, documentStatus, std::size_t) const`,
    /// but with `documentStatus` equal to `DocumentStatus:kActual`.
    [[nodiscard]]
    std::vector<Document>
    findTopDocuments(std::string_view rawQuery,
                     std::size_t topDocumentsCount = kMaxResultDocumentCount) const;

    /// \brief Same as `findTopDocuments(std::string_view, std::size_t) const`, but execute
    /// according to the [execution][1] `policy`.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/algorithm#Execution_policies
    ///
    template <typename ExecutionPolicy>
    [[nodiscard]]
    std::vector<Document>
    findTopDocuments(const ExecutionPolicy& policy,
                     std::string_view rawQuery,
                     std::size_t topDocumentsCount = kMaxResultDocumentCount) const;

    using MatchingWordsAndDocStatus = std::tuple<std::vector<std::string_view>, DocumentStatus>;

    /// \brief Match the document with `documentId` to `rawQuery`.
    ///
    /// \return the pair of [`std::vector`][1] of matched words and the status of the document.
    ///
    /// \exception std::invalid_argument if `documentId` is negative or doesn't exist.
    /// \exception std::invalid_argument if `rawQuery` contains forbidden characters from `0x00` to
    /// `0x1F`.
    /// \exception std::invalid_argument if `rawQuery` contains a word with one character `-`
    /// or a word started with two consecutive `-`.
    ///
    /// \sa [std::invalid_argument](https://en.cppreference.com/w/cpp/error/invalid_argument)
    ///
    /// [1]: https://en.cppreference.com/w/cpp/container/vector
    ///
    [[nodiscard]]
    MatchingWordsAndDocStatus matchDocument(std::string_view rawQuery, int documentId) const;

    /// \brief Effectively call `matchDocument(std::string_view, int) const`.
    [[nodiscard]]
    MatchingWordsAndDocStatus matchDocument(const std::execution::sequenced_policy&,
                                            std::string_view rawQuery,
                                            int documentId) const;

    /// \brief Same as `matchDocument(std::string_view, int) const`, but execute according
    /// to the [std::execution::par][1] parallel policy.
    ///
    /// [1]: https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag
    ///
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

    double computeInverseDocumentFrequency(std::size_t nDocsWithWord) const noexcept;

private: // Parsing

    [[nodiscard]]
    std::vector<std::string_view> splitIntoWordsNoStop(std::string_view text) const;

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
    std::vector<Document> findAllDocuments(const std::execution::parallel_policy& parPolicy,
                                           const Query& query,
                                           Predicate predicate) const;

    template <typename ExecutionPolicy, typename Map, typename Predicate>
    void computeDocumentsRelevance(const ExecutionPolicy& policy,
                                   Map& documentToRelevance,
                                   const Query& query,
                                   Predicate predicate) const;

    [[nodiscard]]
    std::vector<Document>
    prepareResult(const std::unordered_map<int, double>& documentToRelevance) const;
};

/// \brief Remove duplicate documents from `server`
///
/// \param[in] server SearchServer from which to remove duplicate documents
/// \param[out] removedIds the optional reference to [`std::vector`][1] to put in it removed
/// documents' ids
///
/// [1]: https://en.cppreference.com/w/cpp/container/vector
///
SEARCH_SERVER_EXPORT
void removeDuplicates(
        SearchServer& server,
        std::optional<std::reference_wrapper<std::vector<int>>> removedIds = std::nullopt);

} // namespace search_server

#define SEARCH_SERVER_DETAILS_SEARCH_SERVER_INL_HPP_
#include <search_server/details/search_server-inl.hpp>
#undef SEARCH_SERVER_DETAILS_SEARCH_SERVER_INL_HPP_

#endif // SEARCH_SERVER_SEARCH_SERVER_HPP_
