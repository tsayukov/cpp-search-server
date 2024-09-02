#include "generator.hpp"

#include <benchmarking/benchmarking.hpp>

#include <execution>
#include <iostream>
#include <string_view>

using namespace search_server;

template <typename ExecutionPolicy>
void RemoveDocumentImpl(std::string_view mark, const ExecutionPolicy& policy) {
    SearchServer search_server = generator::const_search_server;
    std::cerr << "Benchmarking of " << mark <<" RemoveDocument:\n";
    {
        LOG_DURATION(mark);
        const int document_count = search_server.GetDocumentCount();
        for (int id = 0; id < document_count; ++id) {
            search_server.RemoveDocument(policy, id);
        }
        std::cout << search_server.GetDocumentCount() << std::endl;
    }
}

RUN(RemoveDocument) {
    RemoveDocumentImpl("seq", std::execution::seq);
    RemoveDocumentImpl("par", std::execution::par);
}

template <typename ExecutionPolicy>
void MatchDocumentImpl(std::string_view mark, const ExecutionPolicy& policy) {
    const SearchServer& search_server = generator::const_search_server;
    std::cerr << "Benchmarking of " << mark <<" MatchDocument:\n";
    {
        LOG_DURATION(mark);
        const int document_count = search_server.GetDocumentCount();
        int word_count = 0;
        for (int id = 0; id < document_count; ++id) {
            const auto& [words, status] = search_server.MatchDocument(
                    policy,
                    generator::SearchServerGenerator::query,
                    id);
            word_count += words.size();
        }
        std::cout << word_count << std::endl;
    }
}

RUN(MatchDocument) {
    MatchDocumentImpl("seq", std::execution::seq);
    MatchDocumentImpl("par", std::execution::par);
}

template <typename ExecutionPolicy>
void FindTopDocumentsImpl(std::string_view mark, const ExecutionPolicy& policy) {
    const SearchServer& search_server = generator::const_search_server;
    std::cerr << "Benchmarking of " << mark <<" FindTopDocuments:\n";
    const auto queries = generator::GenerateQueries(
            generator::SearchServerGenerator::dictionary, 100, 70);
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

RUN(FindTopDocuments) {
    FindTopDocumentsImpl("seq", std::execution::seq);
    FindTopDocumentsImpl("par", std::execution::par);
}
