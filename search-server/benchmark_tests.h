#pragma once

#include "log_duration.h"
#include "unit_test_tools.h"
#include "search_server.h"

#include <execution>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>

namespace benchmark_tests {

using namespace std::string_literals;
using namespace unit_test_tools;

inline std::string GenerateWord(std::size_t max_length) {
    assert(max_length > 0);

    const auto length = Generator<std::size_t>::Get(1, max_length);
    std::string word;
    word.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        word.push_back(Generator<char>::Get());
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
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

inline std::string GenerateQuery(const std::vector<std::string>& dictionary,
                                 std::size_t word_count, double minus_prob = 0) {
    assert(word_count > 0);

    std::string query;
    for (std::size_t i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (Generator<double>::Get(0, 1) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[Generator<std::size_t>::Get(0, dictionary.size() - 1)];
    }
    return query;
}

inline std::vector<std::string> GenerateQueries(const std::vector<std::string>& dictionary,
                                                std::size_t query_count, std::size_t max_word_count) {
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

inline const SearchServer const_search_server = [] {
    SearchServer search_server(SearchServerGenerator::dictionary[0]);
    const std::vector<int> ratings = {1, 2, 3};
    for (std::size_t i = 0; i < SearchServerGenerator::documents.size(); ++i) {
        int document_id = static_cast<int>(i);
        search_server.AddDocument(document_id, SearchServerGenerator::documents[i], DocumentStatus::ACTUAL, ratings);
    }
    return search_server;
}();

// BENCHMARK TESTS

template<typename ExecutionPolicy>
void TestRemoveDocument(std::string_view mark, const ExecutionPolicy& policy) {
    SearchServer search_server = const_search_server;
    std::cerr << "Benchmarking of "s << mark <<" RemoveDocument:\n"s;
    {
        LOG_DURATION(mark);
        const int document_count = search_server.GetDocumentCount();
        for (int id = 0; id < document_count; ++id) {
            search_server.RemoveDocument(policy, id);
        }
        std::cout << search_server.GetDocumentCount() << std::endl;
    }
}

template<typename ExecutionPolicy>
void TestMatchDocument(std::string_view mark, const ExecutionPolicy& policy) {
    const SearchServer& search_server = const_search_server;
    std::cerr << "Benchmarking of "s << mark <<" MatchDocument:\n"s;
    {
        LOG_DURATION(mark);
        const int document_count = search_server.GetDocumentCount();
        int word_count = 0;
        for (int id = 0; id < document_count; ++id) {
            const auto& [words, status] = search_server.MatchDocument(policy, SearchServerGenerator::query, id);
            word_count += words.size();
        }
        std::cout << word_count << std::endl;
    }
}

template<typename ExecutionPolicy>
void TestFindTopDocuments(std::string_view mark, const ExecutionPolicy& policy) {
    const SearchServer& search_server = const_search_server;
    std::cerr << "Benchmarking of "s << mark <<" FindTopDocuments:\n"s;
    const auto queries = GenerateQueries(SearchServerGenerator::dictionary, 100, 70);
    {
        LOG_DURATION(mark);
        double total_relevance = 0.0;
        for (const auto& query : queries) {
            for (const auto& document : search_server.FindTopDocuments(policy, query)) {
                total_relevance += document.relevance;
            }
        }
        std::cout << total_relevance << std::endl;
    }
}

} // namespace benchmark_tests

inline void RunAllBenchmarkTests() {
    using namespace benchmark_tests;

    TestRemoveDocument("seq", std::execution::seq);
    TestRemoveDocument("par", std::execution::par);

    TestMatchDocument("seq", std::execution::seq);
    TestMatchDocument("par", std::execution::par);

    TestFindTopDocuments("seq", std::execution::seq);
    TestFindTopDocuments("par", std::execution::par);
}