/// \file search_server.cpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#include <search_server/search_server.hpp>

#include <execution>
#include <numeric>
#include <stdexcept>

namespace search_server {

// Constructors

SearchServer::SearchServer(std::string_view stopWords)
        : SearchServer(details::splitIntoWords(stopWords)) {}

// Capacity

int SearchServer::getDocumentCount() const noexcept {
    return static_cast<int>(mDocuments.size());
}

// Lookup

const std::map<std::string_view, double>& SearchServer::getWordFrequencies(int documentId) const {
    static const std::map<std::string_view, double> emptyMap;

    if (auto it = mDocuments.find(documentId); it != mDocuments.end()) {
        return it->second.wordFrequencies;
    }
    return emptyMap;
}

// Iterators

std::set<int>::const_iterator SearchServer::begin() const noexcept {
    return mDocumentIds.begin();
}

std::set<int>::const_iterator SearchServer::end() const noexcept {
    return mDocumentIds.end();
}

// Modifiers

void SearchServer::addDocument(int documentId,
                               std::string_view document,
                               DocumentStatus status,
                               const std::vector<int>& ratings) {
    checkDocumentIdIsNotNegative(documentId);
    checkDocumentIdDoesntExist(documentId);

    const auto words = splitIntoWordsNoStop(document);

    mDocumentIds.insert(documentId);
    auto& documentData = mDocuments[documentId];
    documentData.rating = computeAverageRating(ratings);
    documentData.status = status;

    const double invSize = 1.0 / static_cast<double>(words.size());
    for (const auto wordView : words) {
        auto iter = mWordToDocumentFrequencies.find(wordView);
        if (iter == mWordToDocumentFrequencies.end()) {
            auto docFrequency = std::unordered_map<int, double>{{documentId, invSize}};
            auto [insertPos, _] =
                    mWordToDocumentFrequencies.emplace(wordView, std::move(docFrequency));
            iter = insertPos;
        } else {
            auto& docFrequency = iter->second;
            docFrequency[documentId] += invSize;
        }
        const std::string_view classWordView = iter->first;
        documentData.wordFrequencies[classWordView] += invSize;
    }
}

void SearchServer::removeDocument(int documentId) {
    if (auto docIter = mDocuments.find(documentId); docIter != mDocuments.end()) {
        const auto& documentData = docIter->second;
        for (const auto& [wordView, _] : documentData.wordFrequencies) {
            auto wordIter = mWordToDocumentFrequencies.find(wordView);
            auto& documentsWithThatWord = wordIter->second;
            documentsWithThatWord.erase(documentId);

            if (documentsWithThatWord.empty()) {
                mWordToDocumentFrequencies.erase(wordIter);
            }
        }

        mDocuments.erase(docIter);
        mDocumentIds.erase(documentId);
    }
}

void SearchServer::removeDocument(const std::execution::sequenced_policy&, int documentId) {
    removeDocument(documentId);
}

void SearchServer::removeDocument(const std::execution::parallel_policy&, int documentId) {
    auto documentIter = mDocuments.find(documentId);
    if (documentIter == mDocuments.end()) {
        return;
    }

    const auto& documentData = documentIter->second;

    std::vector<std::string_view> wordViews(documentData.wordFrequencies.size());
    std::transform(documentData.wordFrequencies.cbegin(), documentData.wordFrequencies.cend(),
                   wordViews.begin(), [](const auto& keyValue) { return keyValue.first; });

    std::for_each(std::execution::par, wordViews.cbegin(), wordViews.cend(),
                  [this, documentId](const auto wordView) {
                      auto& documentsWithThatWord =
                              mWordToDocumentFrequencies.find(wordView)->second;
                      documentsWithThatWord.erase(documentId);
                  });

    mDocuments.erase(documentIter);
    mDocumentIds.erase(documentId);
}

// Search

std::vector<Document> SearchServer::findTopDocuments(std::string_view rawQuery,
                                                     DocumentStatus documentStatus) const {
    return findTopDocuments(rawQuery,
                            [documentStatus](int /*documentId*/, DocumentStatus status,
                                             int /*rating*/) { return status == documentStatus; });
}

std::vector<Document> SearchServer::findTopDocuments(std::string_view rawQuery) const {
    return findTopDocuments(rawQuery, DocumentStatus::kActual);
}

SearchServer::MatchingWordsAndDocStatus SearchServer::matchDocument(std::string_view rawQuery,
                                                                    int documentId) const {
    checkDocumentIdIsNotNegative(documentId);
    checkDocumentIdExists(documentId);

    const auto query = parseQuery(std::execution::seq, rawQuery, WordsRepeatable::kNo);
    const auto& documentData = mDocuments.at(documentId);
    const auto& wordFrequenciesInThatDocuments = documentData.wordFrequencies;

    std::vector<std::string_view> matchedWords;
    for (const auto minusWordView : query.minusWords) {
        if (wordFrequenciesInThatDocuments.count(minusWordView) != 0) {
            return make_tuple(std::move(matchedWords), documentData.status);
        }
    }
    for (const auto plusWordView : query.plusWords) {
        if (wordFrequenciesInThatDocuments.count(plusWordView) != 0) {
            matchedWords.emplace_back(plusWordView);
        }
    }

    return make_tuple(std::move(matchedWords), documentData.status);
}

SearchServer::MatchingWordsAndDocStatus
SearchServer::matchDocument(const std::execution::sequenced_policy&,
                            std::string_view rawQuery,
                            int documentId) const {
    return matchDocument(rawQuery, documentId);
}

SearchServer::MatchingWordsAndDocStatus
SearchServer::matchDocument(const std::execution::parallel_policy&,
                            std::string_view rawQuery,
                            int documentId) const {
    checkDocumentIdIsNotNegative(documentId);
    checkDocumentIdExists(documentId);

    const auto query = parseQuery(std::execution::par, rawQuery, WordsRepeatable::kYes);
    const auto& documentData = mDocuments.at(documentId);
    const auto& wordFrequenciesInThatDocuments = documentData.wordFrequencies;

    std::vector<std::string_view> matchedWords;
    const auto isThatDocumentHasWord = [&wordFrequenciesInThatDocuments](const auto wordView) {
        return wordFrequenciesInThatDocuments.count(wordView) != 0;
    };

    const bool thatDocumentHasMinusWord =
            std::any_of(query.minusWords.begin(), query.minusWords.end(), isThatDocumentHasWord);
    if (thatDocumentHasMinusWord) {
        return make_tuple(std::move(matchedWords), documentData.status);
    }

    matchedWords.resize(query.plusWords.size());
    auto beginOfMatchedWordsToRemove = std::copy_if(query.plusWords.begin(), query.plusWords.end(),
                                                    matchedWords.begin(), isThatDocumentHasWord);

    matchedWords.erase(beginOfMatchedWordsToRemove, matchedWords.end());
    details::removeDuplicateWords(std::execution::par, matchedWords);

    return make_tuple(std::move(matchedWords), documentData.status);
}

std::vector<Document>
SearchServer::prepareResult(const std::unordered_map<int, double>& documentToRelevance) const {
    std::vector<Document> result;
    result.reserve(documentToRelevance.size());
    for (const auto [documentId, relevance] : documentToRelevance) {
        result.emplace_back(documentId, relevance, mDocuments.at(documentId).rating);
    }
    return result;
}

// Checks

void SearchServer::stringHasNotAnyForbiddenChars(std::string_view s) {
    if (std::any_of(s.begin(), s.end(),
                    [](const char c) { return iscntrl(static_cast<unsigned char>(c)); })) {
        throw std::invalid_argument("Stop words contain forbidden characters from 0x00 to 0x1F");
    }
}

void SearchServer::checkDocumentIdIsNotNegative(int documentId) {
    if (documentId < 0) {
        throw std::invalid_argument("The negative document id");
    }
}

void SearchServer::checkDocumentIdDoesntExist(int documentId) const {
    if (mDocuments.count(documentId) > 0) {
        throw std::invalid_argument("The passed document id already exists");
    }
}

void SearchServer::checkDocumentIdExists(int documentId) const {
    if (mDocuments.count(documentId) == 0) {
        throw std::invalid_argument("The passed document id doesn't exist");
    }
}

bool SearchServer::isStopWord(std::string_view word) const {
    return mStopWords.count(word) > 0;
}

// Metric computation

int SearchServer::computeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    const int ratingSum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return ratingSum / static_cast<int>(ratings.size());
}

double SearchServer::computeInverseDocumentFrequency(std::size_t nDocsWithWord) const noexcept {
    return std::log(getDocumentCount() / static_cast<double>(nDocsWithWord));
}

// Parsing

std::vector<std::string_view> SearchServer::splitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string_view> result;
    const auto words = details::splitIntoWordsView(text);
    for (const auto wordView : words) {
        stringHasNotAnyForbiddenChars(wordView);
        if (!isStopWord(wordView)) {
            result.emplace_back(wordView);
        }
    }
    return result;
}

SearchServer::QueryWord SearchServer::parseQueryWord(std::string_view word) const {
    bool isMinus = (word[0] == '-');
    if (isMinus) {
        if (word.size() == 1 || word[1] == '-') {
            throw std::invalid_argument("Incorrect form of minus words");
        }
        word = word.substr(1);
    }
    return QueryWord{word, isMinus, isStopWord(word)};
}

// Modifier

void removeDuplicates(SearchServer& server,
                      std::optional<std::reference_wrapper<std::vector<int>>> removedIds) {
    using namespace std::string_literals;

    std::set<int> idsToRemove;
    std::set<std::set<std::string_view>> documents;
    for (const auto id : server) {
        std::set<std::string_view> words;
        const auto& wordFrequencies = server.getWordFrequencies(id);
        std::transform(wordFrequencies.begin(), wordFrequencies.end(),
                       std::inserter(words, words.end()),
                       [](const auto& keyValue) { return keyValue.first; });
        if (documents.count(words) == 0) {
            documents.insert(words);
        } else {
            idsToRemove.insert(id);
        }
    }

    for (const auto& id : idsToRemove) {
        server.removeDocument(id);
    }

    if (removedIds) {
        removedIds.value().get().assign(idsToRemove.begin(), idsToRemove.end());
    }
}

} // namespace search_server
