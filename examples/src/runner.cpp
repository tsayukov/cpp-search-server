#include <runner.hpp>

#include <search_server/search_server.hpp>

#include <iostream>

namespace runner {

void run() {
    using namespace search_server;

    SearchServer server("and in with");
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(5, "nobody lives in the house", DocumentStatus::kActual, ratings);
    server.AddDocument(2, "cat lives in the house", DocumentStatus::kActual, ratings);
    server.AddDocument(6, "cat and dog live in the house", DocumentStatus::kActual, ratings);
    server.AddDocument(4, "cat and dog and bird live in the house", DocumentStatus::kActual, ratings);
    auto found_docs = server.FindTopDocuments("cat dog bird");
    std::cout << "Found: " << found_docs.size() << " documents." << std::endl;
}

} // namespace runner
