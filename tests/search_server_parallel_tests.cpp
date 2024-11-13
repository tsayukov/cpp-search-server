#include <search_server/search_server.hpp>

#include <testing/testing.hpp>

using namespace search_server;
using namespace std::string_literals;
using namespace std::string_view_literals;

TEST(ParRemoveDocument) {
    SearchServer server("and in with"sv);
    {
        server.removeDocument(std::execution::par, 1);
        ASSERT_EQUAL(server.getDocumentCount(), 0);
        server.removeDocument(std::execution::par, 100);
        ASSERT_EQUAL(server.getDocumentCount(), 0);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.addDocument(0, "white cat"sv, DocumentStatus::kActual, ratings);
    server.addDocument(1, "black cat"sv, DocumentStatus::kActual, ratings);
    server.addDocument(5, "blue cat"sv, DocumentStatus::kActual, ratings);
    server.addDocument(10, "another blue cat"sv, DocumentStatus::kActual, ratings);
    server.addDocument(100, "blue cat and blue kitty"sv, DocumentStatus::kActual, ratings);
    {
        server.removeDocument(std::execution::par, 2);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 1, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
    {
        server.removeDocument(std::execution::par, 1);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
}

TEST(ParMatchingDocuments) {
    const int docId = 42;
    const auto content = "cats in the city of cats"sv;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.addDocument(docId, content, DocumentStatus::kActual, ratings);
    {
        auto [matchWords, _] =
                server.matchDocument(std::execution::par, "beautiful cats city"sv, docId);
        ASSERT_EQUAL_HINT(matchWords.size(), 2u,
                          "Matching words between a content of the document "
                          "and the query without repetitions: "
                          "checks size of matched words"s);
        sort(matchWords.begin(), matchWords.end());
        const std::vector<std::string_view> answer = {"cats"sv, "city"sv};
        ASSERT_EQUAL_HINT(matchWords, answer,
                          "Matching words between a content of the document "
                          "and the query without repetitions: "
                          "checks matched words"s);
    }
    {
        const auto [matchWords, _] =
                server.matchDocument(std::execution::par, "cats -city"sv, docId);
        ASSERT_HINT(matchWords.empty(), "Minus words must be excluded from matching"s);
    }
}

TEST(ParFindTopDocumentsWithPredicate) {
    SearchServer server(""sv);
    server.addDocument(1, "nobody lives in the house"sv, DocumentStatus::kIrrelevant, {0});
    server.addDocument(2, "cat lives in the house"sv, DocumentStatus::kActual, {5});
    server.addDocument(3, "cat and dog live in the house"sv, DocumentStatus::kActual, {5});
    server.addDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::kActual, {4});
    {
        auto foundDocs = server.findTopDocuments(std::execution::par, "cat dog bird"sv,
                                                 [](auto id, auto, auto) { return id >= 3; });
        ASSERT_EQUAL(foundDocs[0].id, 4);
        ASSERT_EQUAL(foundDocs[1].id, 3);
    }
    {
        auto foundDocs =
                server.findTopDocuments(std::execution::par, "cat dog bird"sv,
                                        [](auto, auto, auto rating) { return rating == 5; });
        ASSERT_EQUAL(foundDocs[0].id, 3);
        ASSERT_EQUAL(foundDocs[1].id, 2);
    }
}

TEST(ParFindTopDocumentsWithSpecifiedStatus) {
    SearchServer server(""sv);
    server.addDocument(1, "nobody lives in the house"sv, DocumentStatus::kIrrelevant, {0});
    server.addDocument(2, "cat lives in the house"sv, DocumentStatus::kBanned, {5});
    server.addDocument(3, "cat and dog live in the house"sv, DocumentStatus::kRemoved, {5});
    server.addDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::kActual, {4});
    {
        auto foundDocs = server.findTopDocuments(std::execution::par, "house"sv,
                                                 DocumentStatus::kIrrelevant);
        ASSERT_EQUAL_HINT(foundDocs[0].id, 1,
                          "A relevant document with a specified status must be found"s);
    }
    {
        auto foundDocs =
                server.findTopDocuments(std::execution::par, "house"sv, DocumentStatus::kBanned);
        ASSERT_EQUAL_HINT(foundDocs[0].id, 2,
                          "A relevant document with a specified status must be found"s);
    }
    {
        auto foundDocs =
                server.findTopDocuments(std::execution::par, "house"sv, DocumentStatus::kRemoved);
        ASSERT_EQUAL_HINT(foundDocs[0].id, 3,
                          "A relevant document with a specified status must be found"s);
    }
    {
        auto foundDocs =
                server.findTopDocuments(std::execution::par, "house"sv, DocumentStatus::kActual);
        ASSERT_EQUAL_HINT(foundDocs[0].id, 4,
                          "A relevant document with a specified status must be found"s);
    }
}
