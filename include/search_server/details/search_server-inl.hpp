#ifndef SEARCH_SERVER_DETAILS_SEARCH_SERVER_INL_HPP_
    #error "`search_server.hpp.inc` cannot be included outside of `search_server.hpp`"
#endif

#include <search_server/search_server.hpp>

namespace search_server {

// Constructor

template <typename Container, std::enable_if_t<details::kContainsStringViewLike<Container>, int>>
SearchServer::SearchServer(Container&& stopWords) {
    for (auto& word : stopWords) {
        if (!word.empty() && (stringHasNotAnyForbiddenChars(word), true)) {
            mStopWords.insert(std::string(std::move(word)));
        }
    }
}

// Parsing

template <typename ExecutionPolicy>
SearchServer::Query SearchServer::parseQuery(const ExecutionPolicy& policy,
                                             std::string_view text,
                                             WordsRepeatable wordsCanBeRepeated) const {
    Query query;
    const auto words = details::splitIntoWordsView(text);
    for (const auto word : words) {
        stringHasNotAnyForbiddenChars(word);
        const auto queryWord = parseQueryWord(word);
        if (!(queryWord.isStop)) {
            if (queryWord.isMinus) {
                query.minusWords.push_back(queryWord.content);
            } else {
                query.plusWords.push_back(queryWord.content);
            }
        }
    }

    if (static_cast<bool>(wordsCanBeRepeated)) {
        return query;
    }

    details::removeDuplicateWords(policy, query.plusWords);
    details::removeDuplicateWords(policy, query.minusWords);

    return query;
}

// Search

template <typename Predicate>
std::vector<Document> SearchServer::findTopDocuments(std::string_view rawQuery,
                                                     Predicate predicate) const {
    return findTopDocuments(std::execution::seq, rawQuery, predicate);
}

template <typename ExecutionPolicy, typename Predicate>
std::vector<Document> SearchServer::findTopDocuments(const ExecutionPolicy& policy,
                                                     std::string_view rawQuery,
                                                     Predicate predicate) const {
    auto result =
            findAllDocuments(policy, parseQuery(policy, rawQuery, WordsRepeatable::kNo), predicate);

    std::sort(policy, result.begin(), result.end(), [](Document lhs, Document rhs) noexcept {
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
std::vector<Document> SearchServer::findTopDocuments(const ExecutionPolicy& policy,
                                                     std::string_view rawQuery,
                                                     DocumentStatus documentStatus) const {
    return findTopDocuments(
            policy, rawQuery,
            [documentStatus](int /*documentId*/, DocumentStatus status, int /*rating*/) noexcept {
                return status == documentStatus;
            });
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::findTopDocuments(const ExecutionPolicy& policy,
                                                     std::string_view rawQuery) const {
    return findTopDocuments(policy, rawQuery, DocumentStatus::kActual);
}

template <typename Predicate>
std::vector<Document> SearchServer::findAllDocuments(const Query& query,
                                                     Predicate predicate) const {
    std::unordered_map<int, double> docToRelevance;
    computeDocumentsRelevance(std::execution::seq, docToRelevance, query, predicate);
    return prepareResult(docToRelevance);
}

template <typename Predicate>
std::vector<Document> SearchServer::findAllDocuments(const std::execution::sequenced_policy&,
                                                     const Query& query,
                                                     Predicate predicate) const {
    return findAllDocuments(query, predicate);
}

template <typename Predicate>
std::vector<Document>
SearchServer::findAllDocuments(const std::execution::parallel_policy& parPolicy,
                               const Query& query,
                               Predicate predicate) const {
    details::ConcurrentMap<int, double> concurrentDocToRelevance(
            std::thread::hardware_concurrency());
    computeDocumentsRelevance(parPolicy, concurrentDocToRelevance, query, predicate);
    return prepareResult(concurrentDocToRelevance.buildOrdinaryMap());
}

template <typename ExecutionPolicy, typename Map, typename Predicate>
void SearchServer::computeDocumentsRelevance(const ExecutionPolicy& policy,
                                             Map& documentToRelevance,
                                             const Query& query,
                                             Predicate predicate) const {
    static_assert(std::is_integral_v<typename Map::key_type>
                  && std::is_floating_point_v<typename Map::mapped_type>);

    std::for_each(policy, query.plusWords.begin(), query.plusWords.end(),
                  [this, predicate, &documentToRelevance](auto plusWordView) {
                      auto iter = mWordToDocumentFrequencies.find(plusWordView);
                      if (iter == mWordToDocumentFrequencies.end()) {
                          return;
                      }
                      const auto& documentFrequencies = iter->second;

                      // Computation TF-IDF (term frequency–inverse document
                      // frequency) source:
                      // https://en.wikipedia.org/wiki/Tf%E2%80%93idf
                      const double idf =
                              computeInverseDocumentFrequency(documentFrequencies.size());
                      for (const auto [documentId, tf] : documentFrequencies) {
                          const auto& documentData = mDocuments.at(documentId);
                          if (predicate(documentId, documentData.status, documentData.rating)) {
                              documentToRelevance[documentId] += tf * idf;
                          }
                      }
                  });

    std::for_each(policy, query.minusWords.begin(), query.minusWords.end(),
                  [this, &documentToRelevance](auto minusWordView) {
                      auto iter = mWordToDocumentFrequencies.find(minusWordView);
                      if (iter == mWordToDocumentFrequencies.end()) {
                          return;
                      }
                      const auto& documentFrequencies = iter->second;
                      for (const auto [documentId, _] : documentFrequencies) {
                          documentToRelevance.erase(documentId);
                      }
                  });
}

} // namespace search_server
