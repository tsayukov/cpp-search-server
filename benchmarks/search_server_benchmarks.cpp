#include "generator.hpp"

#include "search_server/search_server.hpp"

#include <chrono>
#include <execution>
#include <iostream>
#include <string_view>

#include <benchmarking/benchmarking.hpp>
#include <testing/assertions.hpp>

using namespace search_server;

using ms = std::chrono::milliseconds;
using namespace std::chrono_literals;

template <typename ExecutionPolicy>
auto matchDocumentImpl(const ExecutionPolicy& policy,
                       const search_server::SearchServer& searchServer,
                       const std::vector<std::string>& queries) {
    const int documentCount = searchServer.getDocumentCount();
    int wordCount = 0;
    ms duration = 0ms;
    ms durationPerQuery = 0ms;
    for (std::size_t i = 0; i < queries.size(); ++i, duration += durationPerQuery) {
        LOG_DURATION(&durationPerQuery);
        for (int id = 0; id < documentCount; ++id) {
            const auto& [words, _] = searchServer.matchDocument(policy, queries[i], id);
            wordCount += words.size();
        }
    }

    // To prevent optimization of the benchmarking code
    std::cout << "Matched words' count: " << wordCount << std::endl;

    return static_cast<double>(duration.count());
}

RUN(MatchDocument) {
    const auto& searchServer = generator::kConstSearchServer;
    const auto queries = generator::generateQueries(generator::kDictionary, 10, 500);

    auto par = matchDocumentImpl(std::execution::par, searchServer, queries);
    std::cout << "MatchDocument (par): " << par << '\n';
    auto seq = matchDocumentImpl(std::execution::seq, searchServer, queries);
    std::cout << "MatchDocument (seq): " << seq << '\n';
    auto quot = seq / par;
    std::cout << "Quot: " << quot << std::endl;
    ASSERT(quot - 1.5 >= 1e-6);
}

template <typename ExecutionPolicy>
auto removeDocumentImpl(const ExecutionPolicy& policy) {
    auto searchServer = generator::generateSearchServer(100'000, 70);
    const int documentCount = searchServer.getDocumentCount();
    ms duration;
    {
        LOG_DURATION(&duration);
        for (int id = 0; id < documentCount; ++id) {
            searchServer.removeDocument(policy, id);
        }
    }

    // To prevent optimization of the benchmarking code
    std::cout << "Document's count: " << searchServer.getDocumentCount() << std::endl;

    return static_cast<double>(duration.count());
}

RUN(RemoveDocument) {
    auto par = removeDocumentImpl(std::execution::par);
    std::cout << "RemoveDocument (par): " << par << '\n';
    auto seq = removeDocumentImpl(std::execution::seq);
    std::cout << "RemoveDocument (seq): " << seq << '\n';
    auto quot = seq / par;
    std::cout << "Quot: " << quot << std::endl;
    ASSERT(quot - 1.5 >= 1e-6);
}

template <typename ExecutionPolicy>
auto findTopDocumentsImpl(const ExecutionPolicy& policy,
                          const search_server::SearchServer& searchServer,
                          const std::vector<std::string>& queries) {
    ms duration;
    std::vector<std::vector<search_server::Document>> result;
    result.reserve(queries.size());
    {
        LOG_DURATION(&duration);
        for (const auto& query : queries) {
            result.emplace_back(searchServer.findTopDocuments(policy, query));
        }
    }

    // To prevent optimization of the benchmarking code
    double totalRelevance = 0.0;
    for (const auto& documents : result) {
        for (const auto& doc : documents) {
            totalRelevance += doc.relevance;
        }
    }
    std::cout << "Total relevance: " << totalRelevance << std::endl;

    return static_cast<double>(duration.count());
}

RUN(FindTopDocuments) {
    const auto& searchServer = generator::kConstSearchServer;
    const auto queries = generator::generateQueries(generator::kDictionary, 1'000, 500);

    auto par = findTopDocumentsImpl(std::execution::par, searchServer, queries);
    std::cout << "FindTopDocuments (par): " << par << '\n';
    auto seq = findTopDocumentsImpl(std::execution::seq, searchServer, queries);
    std::cout << "FindTopDocuments (seq): " << seq << '\n';
    auto quot = seq / par;
    std::cout << "Quot: " << quot << std::endl;
    ASSERT(quot - 1.5 >= 1e-6);
}
