#include "generator.hpp"

#include <execution>
#include <iostream>
#include <string_view>

#include <benchmarking/benchmarking.hpp>

using namespace search_server;

template <typename ExecutionPolicy>
void removeDocumentImpl(std::string_view mark, const ExecutionPolicy& policy) {
    SearchServer searchServer = generator::kConstSearchServer;
    std::cerr << "Benchmarking of " << mark << " removeDocument:\n";
    {
        LOG_DURATION(mark);
        const int documentCount = searchServer.getDocumentCount();
        for (int id = 0; id < documentCount; ++id) {
            searchServer.removeDocument(policy, id);
        }
        std::cout << searchServer.getDocumentCount() << std::endl;
    }
}

RUN(removeDocument) {
    removeDocumentImpl("seq", std::execution::seq);
    removeDocumentImpl("par", std::execution::par);
}

template <typename ExecutionPolicy>
void matchDocumentImpl(std::string_view mark, const ExecutionPolicy& policy) {
    const SearchServer& searchServer = generator::kConstSearchServer;
    std::cerr << "Benchmarking of " << mark << " matchDocument:\n";
    {
        LOG_DURATION(mark);
        const int documentCount = searchServer.getDocumentCount();
        int wordCount = 0;
        for (int id = 0; id < documentCount; ++id) {
            const auto& [words, status] =
                    searchServer.matchDocument(policy, generator::SearchServerGenerator::query, id);
            wordCount += words.size();
        }
        std::cout << wordCount << std::endl;
    }
}

RUN(matchDocument) {
    matchDocumentImpl("seq", std::execution::seq);
    matchDocumentImpl("par", std::execution::par);
}

template <typename ExecutionPolicy>
void findTopDocumentsImpl(std::string_view mark, const ExecutionPolicy& policy) {
    const SearchServer& searchServer = generator::kConstSearchServer;
    std::cerr << "Benchmarking of " << mark << " findTopDocuments:\n";
    const auto queries =
            generator::GenerateQueries(generator::SearchServerGenerator::dictionary, 100, 70);
    {
        LOG_DURATION(mark);
        double totalRelevance = 0.0;
        for (const auto& query : queries) {
            for (const auto& document : searchServer.findTopDocuments(policy, query)) {
                totalRelevance += document.relevance;
            }
        }
        std::cout << totalRelevance << std::endl;
    }
}

RUN(findTopDocuments) {
    findTopDocumentsImpl("seq", std::execution::seq);
    findTopDocumentsImpl("par", std::execution::par);
}
