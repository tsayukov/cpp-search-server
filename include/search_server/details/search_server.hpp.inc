#ifndef SEARCH_SERVER_DETAILS_SEARCH_SERVER_HPP_INC_
    #error "`search_server.hpp.inc` cannot be included outside of `search_server.hpp`"
#endif

#include <search_server/search_server.hpp>

namespace search_server {

// Constructor

template <typename Container, std::enable_if_t<details::kContainsStringViewLike<Container>, int>>
SearchServer::SearchServer(Container&& stop_words) {
    for (auto& stop_word : stop_words) {
        if (!stop_word.empty() && (StringHasNotAnyForbiddenChars(stop_word), true)) {
            stop_words_.insert(std::string(std::move(stop_word)));
        }
    }
}

// Parsing

template <typename ExecutionPolicy>
[[nodiscard]]
SearchServer::Query SearchServer::ParseQuery(const ExecutionPolicy& policy,
                                             std::string_view text,
                                             WordsRepeatable words_can_be_repeated) const {
    Query query;
    const auto words = details::SplitIntoWordsView(text);
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

template <typename Predicate>
[[nodiscard]]
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                     Predicate predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template <typename ExecutionPolicy, typename Predicate>
[[nodiscard]]
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy,
                                                     std::string_view raw_query,
                                                     Predicate predicate) const {
    auto result = FindAllDocuments(
            policy,
            ParseQuery(policy, raw_query, WordsRepeatable::kNo),
            predicate);

    std::sort(policy,
              result.begin(), result.end(),
              [](const Document& lhs, const Document& rhs) noexcept {
                  if (std::abs(lhs.relevance - rhs.relevance) < kRelevanceErrorMargin) {
                      return lhs.rating > rhs.rating;
                  }
                  return lhs.relevance > rhs.relevance;
              });

    if (result.size() > kMaxResultDocumentCount) {
        result.resize(kMaxResultDocumentCount);
    }
    return result;
}

template <typename ExecutionPolicy>
[[nodiscard]]
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy,
                                                     std::string_view raw_query,
                                                     DocumentStatus document_status) const {
    return FindTopDocuments(
            policy,
            raw_query,
            [document_status](int /*document_id*/, DocumentStatus status, int /*rating*/) noexcept {
                return status == document_status;
            });
}

template <typename ExecutionPolicy>
[[nodiscard]]
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy,
                                                     std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::kActual);
}

template <typename Predicate>
[[nodiscard]]
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
                                                     Predicate predicate) const {
    std::map<int, double> doc_to_relevance;
    ComputeDocumentsRelevance(std::execution::seq, doc_to_relevance, query, predicate);
    return PrepareResult(doc_to_relevance);
}

template <typename Predicate>
[[nodiscard]]
std::vector<Document> SearchServer::FindAllDocuments(
        const std::execution::sequenced_policy&,
        const Query& query,
        Predicate predicate) const {
    return FindAllDocuments(query, predicate);
}

template <typename Predicate>
[[nodiscard]]
std::vector<Document> SearchServer::FindAllDocuments(
        const std::execution::parallel_policy& par_policy,
        const Query& query,
        Predicate predicate) const {
    details::concurrent::Map<int, double> concurrent_doc_to_relevance(std::thread::hardware_concurrency());
    ComputeDocumentsRelevance(par_policy, concurrent_doc_to_relevance, query, predicate);
    return PrepareResult(concurrent_doc_to_relevance.BuildOrdinaryMap());
}

template <typename ExecutionPolicy, typename Map, typename Predicate>
void SearchServer::ComputeDocumentsRelevance(const ExecutionPolicy& policy,
                                             Map& document_to_relevance,
                                             const Query& query,
                                             Predicate predicate) const {
    static_assert(std::is_integral_v<typename Map::key_type>
            && std::is_floating_point_v<typename Map::mapped_type>);

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

} // namespace search_server
