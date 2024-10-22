#ifndef SEARCH_SERVER_GENERATOR_HPP_
#define SEARCH_SERVER_GENERATOR_HPP_

#include <search_server/search_server.hpp>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include <benchmarking/generator.hpp>

namespace search_server::generator {

inline std::string generateWord(std::size_t maxLength) {
    if (maxLength == 0) {
        throw std::invalid_argument("`maxLength` cannot be zero.");
    }

    const auto length = benchmarking::Generator<std::size_t>::get(1, maxLength);
    std::string word;
    word.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        word.push_back(benchmarking::Generator<char>::get());
    }
    return word;
}

inline std::vector<std::string> GenerateDictionary(std::size_t wordCount, std::size_t maxLength) {
    if (wordCount == 0) {
        throw std::invalid_argument("`wordCount` cannot be zero.");
    }

    std::vector<std::string> words;
    words.reserve(wordCount);
    for (std::size_t i = 0; i < wordCount; ++i) {
        words.push_back(generateWord(maxLength));
    }
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
    return words;
}

inline std::string GenerateQuery(const std::vector<std::string>& dictionary,
                                 std::size_t wordCount,
                                 double minusProb = 0) {
    if (wordCount == 0) {
        throw std::invalid_argument("`wordCount` cannot be zero.");
    }

    std::string query;
    for (std::size_t i = 0; i < wordCount; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (benchmarking::Generator<double>::get(0, 1) < minusProb) {
            query.push_back('-');
        }
        query += dictionary[benchmarking::Generator<std::size_t>::get(0, dictionary.size() - 1)];
    }
    return query;
}

inline std::vector<std::string> GenerateQueries(const std::vector<std::string>& dictionary,
                                                std::size_t queryCount,
                                                std::size_t maxWordCount) {
    std::vector<std::string> queries;
    queries.reserve(queryCount);
    for (std::size_t i = 0; i < queryCount; ++i) {
        queries.push_back(GenerateQuery(dictionary, maxWordCount));
    }
    return queries;
}

struct SearchServerGenerator {
    inline static const auto dictionary = GenerateDictionary(1'000, 10);
    inline static const auto documents = GenerateQueries(dictionary, 10'000, 70);
    inline static const auto query = GenerateQuery(dictionary, 500, 0.1);
};

inline const SearchServer kConstSearchServer = std::invoke([] {
    SearchServer searchServer(SearchServerGenerator::dictionary[0]);
    const std::vector<int> ratings = {1, 2, 3};
    for (std::size_t i = 0; i < SearchServerGenerator::documents.size(); ++i) {
        int documentId = static_cast<int>(i);
        searchServer.addDocument(documentId, SearchServerGenerator::documents[i],
                                 DocumentStatus::kActual, ratings);
    }
    return searchServer;
});

} // namespace search_server::generator

#endif // SEARCH_SERVER_GENERATOR_HPP_
