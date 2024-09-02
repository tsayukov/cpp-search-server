#ifndef SEARCH_SERVER_GENERATOR_HPP_
#define SEARCH_SERVER_GENERATOR_HPP_

#include <search_server/search_server.hpp>

#include <benchmarking/generator.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <string>
#include <vector>

namespace search_server::generator {

inline std::string GenerateWord(std::size_t max_length) {
    assert(max_length > 0);

    const auto length = benchmarking::Generator<std::size_t>::Get(1, max_length);
    std::string word;
    word.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        word.push_back(benchmarking::Generator<char>::Get());
    }
    return word;
}

inline std::vector<std::string> GenerateDictionary(std::size_t word_count, std::size_t max_length) {
    assert(word_count > 0);

    std::vector<std::string> words;
    words.reserve(word_count);
    for (std::size_t i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(max_length));
    }
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
    return words;
}

inline std::string GenerateQuery(const std::vector<std::string>& dictionary,
                                 std::size_t word_count,
                                 double minus_prob = 0) {
    assert(word_count > 0);

    std::string query;
    for (std::size_t i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (benchmarking::Generator<double>::Get(0, 1) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[benchmarking::Generator<std::size_t>::Get(0, dictionary.size() - 1)];
    }
    return query;
}

inline std::vector<std::string> GenerateQueries(const std::vector<std::string>& dictionary,
                                                std::size_t query_count,
                                                std::size_t max_word_count) {
    std::vector<std::string> queries;
    queries.reserve(query_count);
    for (std::size_t i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(dictionary, max_word_count));
    }
    return queries;
}

struct SearchServerGenerator {
    inline static const auto dictionary = GenerateDictionary(1'000, 10);
    inline static const auto documents = GenerateQueries(dictionary, 10'000, 70);
    inline static const auto query = GenerateQuery(dictionary, 500, 0.1);
};

inline const SearchServer const_search_server = std::invoke([] {
    SearchServer search_server(SearchServerGenerator::dictionary[0]);
    const std::vector<int> ratings = {1, 2, 3};
    for (std::size_t i = 0; i < SearchServerGenerator::documents.size(); ++i) {
        int document_id = static_cast<int>(i);
        search_server.AddDocument(document_id,
                                  SearchServerGenerator::documents[i],
                                  DocumentStatus::kActual,
                                  ratings);
    }
    return search_server;
});

} // namespace search_server::generator

#endif // SEARCH_SERVER_GENERATOR_HPP_
