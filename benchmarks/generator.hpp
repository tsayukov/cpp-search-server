#ifndef SEARCH_SERVER_GENERATOR_HPP_
#define SEARCH_SERVER_GENERATOR_HPP_

#include <search_server/search_server.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <benchmarking/generator.hpp>

namespace search_server::generator {

namespace bench = benchmarking;

inline std::string generateWord(std::size_t maxLength) {
    if (maxLength == 0) {
        throw std::invalid_argument("`maxLength` cannot be zero.");
    }

    const auto length = bench::Generator<std::size_t>::get(1, maxLength);
    std::string word;
    word.reserve(length);
    std::generate_n(std::back_inserter(word), length,
                    [&] { return bench::Generator<char>::get(); });
    return word;
}

inline std::vector<std::string> generateDictionary(std::size_t wordCount, std::size_t maxLength) {
    if (wordCount == 0) {
        throw std::invalid_argument("`wordCount` cannot be zero.");
    }

    std::vector<std::string> words;
    words.reserve(wordCount);
    std::generate_n(std::back_inserter(words), wordCount, [=] { return generateWord(maxLength); });
    std::sort(std::execution::par, words.begin(), words.end());
    words.erase(std::unique(std::execution::par, words.begin(), words.end()), words.end());
    return words;
}

inline const auto kDictionary = generateDictionary(1'000, 10);

inline std::vector<std::string> generateStopWords(const std::vector<std::string>& dictionary,
                                                  std::size_t wordCount) {
    if (wordCount == 0) {
        throw std::invalid_argument("`wordCount` cannot be zero.");
    }

    std::vector<std::string> stopWords;
    stopWords.reserve(wordCount);
    std::generate_n(std::back_inserter(stopWords), wordCount, [&] {
        const auto index = bench::Generator<std::size_t>::get(0, dictionary.size() - 1);
        return dictionary[index];
    });
    std::sort(stopWords.begin(), stopWords.end());
    stopWords.erase(std::unique(stopWords.begin(), stopWords.end()), stopWords.end());
    return stopWords;
}

inline std::string generateQuery(const std::vector<std::string>& dictionary,
                                 std::size_t maxWordCount,
                                 double minusProb = 0) {
    if (maxWordCount == 0) {
        throw std::invalid_argument("`maxWordCount` cannot be zero.");
    }

    std::string query;
    const auto wordCount = bench::Generator<std::size_t>::get(1, maxWordCount);
    for (std::size_t i = 0; i < wordCount; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (bench::Generator<double>::get(0, 1) < minusProb) {
            query.push_back('-');
        }
        const auto index = bench::Generator<std::size_t>::get(0, dictionary.size() - 1);
        query += dictionary[index];
    }
    return query;
}

inline std::vector<std::string> generateQueries(const std::vector<std::string>& dictionary,
                                                std::size_t queryCount,
                                                std::size_t maxWordCount) {
    std::vector<std::string> queries;
    queries.reserve(queryCount);
    for (std::size_t i = 0; i < queryCount; ++i) {
        queries.push_back(generateQuery(dictionary, maxWordCount, 0.1));
    }
    return queries;
}

inline SearchServer generateSearchServer(std::size_t documentCount, std::size_t maxWordCount) {
    SearchServer searchServer(generateStopWords(kDictionary, 50));
    const auto documents = generateQueries(kDictionary, documentCount, maxWordCount);
    const std::vector<int> ratings = {1, 2, 3};
    for (std::size_t i = 0; i < documents.size(); ++i) {
        int documentId = static_cast<int>(i);
        searchServer.addDocument(documentId, documents[i], DocumentStatus::kActual, ratings);
    }
    return searchServer;
}

inline const SearchServer kConstSearchServer = generateSearchServer(10'000, 70);

} // namespace search_server::generator

#endif // SEARCH_SERVER_GENERATOR_HPP_
