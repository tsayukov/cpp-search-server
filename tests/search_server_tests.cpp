#include <search_server/search_server.hpp>

#include <testing/testing.hpp>

#include <stdexcept>

using namespace search_server;
using namespace std::string_literals;
using namespace std::string_view_literals;

inline constexpr double ERROR_MARGIN = 1e-6;

TEST(Constructors) {
    ASSERT_THROW(SearchServer("in \x12the"sv), std::invalid_argument);
    ASSERT_THROW(SearchServer(std::vector<std::string>{"in"s, "\x12the"s}), std::invalid_argument);
    ASSERT_THROW(SearchServer(std::vector<std::string_view>{"in"sv, "\x12the"sv}), std::invalid_argument);
}

TEST(RangeBasedForLoop) {
    {
        const SearchServer empty_server(""sv);
        std::vector<int> res;
        for (const auto id : empty_server) {
            res.push_back(id);
        }
        ASSERT(res.empty());
    }
    {
        const std::vector<int> ratings = {1, 2, 3};
        SearchServer server("and in with"sv);
        server.AddDocument(0, "white cat"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(1, "black cat"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(5, "blue cat"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(10, "another blue cat"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(100, "blue cat and blue kitty"sv, DocumentStatus::kActual, ratings);
        std::vector<int> res;
        for (const auto id : server) {
            res.push_back(id);
        }
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 1, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
}

TEST(AddDocument) {
    const int doc_id = 42;
    const auto content = "cat in the city"sv;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server(""sv);
        server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
        auto found_docs = server.FindTopDocuments("in"sv);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server(""sv);
        ASSERT_THROW(
                server.AddDocument(doc_id, "cat in \x12the city"sv, DocumentStatus::kActual, ratings),
                std::invalid_argument);
        auto found_docs = server.FindTopDocuments("in"sv);
        ASSERT(found_docs.empty());
    }
}

TEST(RemoveDocument) {
    SearchServer server("and in with"sv);
    {
        server.RemoveDocument(1);
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        server.RemoveDocument(100);
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(1, "black cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(5, "blue cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(10, "another blue cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(100, "blue cat and blue kitty"sv, DocumentStatus::kActual, ratings);
    {
        server.RemoveDocument(2);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 1, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
    {
        server.RemoveDocument(1);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
}

TEST(GetWordFrequencies) {
    SearchServer server("and in with"sv);
    {
        ASSERT(server.GetWordFrequencies(1).empty());
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(1, "black cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(5, "blue cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(10, "another blue cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(100, "blue cat and blue kitty"sv, DocumentStatus::kActual, ratings);
    {
        std::map<std::string_view, double> answer = {{"white"sv, 1.0 / 2}, {"cat"sv, 1.0 / 2}};
        ASSERT_EQUAL(server.GetWordFrequencies(0), answer);
    }
    {
        std::map<std::string_view, double> answer = {{"blue"sv, 2.0 / 4}, {"cat"sv, 1.0 / 4}, {"kitty"sv, 1.0 / 4}};
        ASSERT_EQUAL(server.GetWordFrequencies(100), answer);
    }
}

TEST(ExcludeStopWordsFromAddedDocumentContent) {
    const int doc_id = 42;
    const auto content = "cat in the city"sv;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server("in the"sv);
        server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
        auto found_docs = server.FindTopDocuments("in"sv);
        ASSERT_HINT(found_docs.empty(), "Stop words must be excluded from documents"s);
    }
}

TEST(ExcludeDocumentsWithMinusWords) {
    const int doc_id = 42;
    const auto content = "cat in the city"sv;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
    {
        auto found_docs = server.FindTopDocuments("cat -city -town -village"sv);
        ASSERT_HINT(found_docs.empty(), "Relevant documents with minus words must be excluded from the result"s);

        found_docs = server.FindTopDocuments("cat -dog"sv);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Relevant documents without minus words must be found"s);
    }
    {
        ASSERT_THROW((void) server.FindTopDocuments("cat -"sv), std::invalid_argument);
        ASSERT_THROW((void) server.FindTopDocuments("cat --"sv), std::invalid_argument);
        (void) server.FindTopDocuments("cat-"sv);
        (void) server.FindTopDocuments("cat -dog-"sv);
        (void) server.FindTopDocuments("cat -d-o-g"sv);
        ASSERT_THROW((void) server.FindTopDocuments("ca\x12t"sv), std::invalid_argument);
    }
}

TEST(MatchingDocuments) {
    const int doc_id = 42;
    const auto content = "cats in the city of cats"sv;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.AddDocument(doc_id, content, DocumentStatus::kActual, ratings);
    {
        auto [match_words, _] = server.MatchDocument("beautiful cats city"sv, doc_id);
        ASSERT_EQUAL_HINT(match_words.size(), 2u,
                          "Matching words between a content of the document and the query without repetitions: "
                          "checks size of matched words"s);
        sort(match_words.begin(), match_words.end());
        const std::vector<std::string_view> answer = { "cats"sv, "city"sv };
        ASSERT_EQUAL_HINT(match_words, answer,
                          "Matching words between a content of the document and the query without repetitions: "
                          "checks matched words"s);
    }
    {
        const auto [match_words, _] = server.MatchDocument("cats -city"sv, doc_id);
        ASSERT_HINT(match_words.empty(), "Minus words must be excluded from matching"s);
    }
}

TEST(SortingDocumentsByRelevance) {
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.AddDocument(5, "nobody lives in the house"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(6, "cat and dog live in the house"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::kActual, ratings);
    auto found_docs = server.FindTopDocuments("cat dog bird"sv);
    std::vector<double> result;
    result.reserve(found_docs.size());
    for (const auto& document : found_docs) {
        result.push_back(document.relevance);
    }
    std::vector<double> answer = result;
    sort(answer.begin(), answer.end(),
         [](double x, double y) {
             return x > y;
         });
    ASSERT_EQUAL(result, answer);
}

TEST(DocumentRating) {
    SearchServer server(""sv);
    server.AddDocument(5, "nobody lives in the house"sv, DocumentStatus::kActual, {});
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::kActual, {5});
    server.AddDocument(3, "dog lives in the house"sv, DocumentStatus::kActual, {5, 5, 5});
    {
        auto found_docs = server.FindTopDocuments("nobody"sv);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 0,
                          "Nobody rated the document, so the rating must be equal to zero"s);
    }
    {
        auto found_docs = server.FindTopDocuments("cat"sv);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 5, "One rating"s);
    }
    {
        auto found_docs = server.FindTopDocuments("dog"sv);
        ASSERT_EQUAL_HINT(found_docs[0].rating, (5 + 5 + 5) / 3, "A few ratings"s);
    }
}

TEST(FindTopDocumentsWithPredicate) {
    SearchServer server(""sv);
    server.AddDocument(1, "nobody lives in the house"sv, DocumentStatus::kIrrelevant, { 0 });
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::kActual, { 5 });
    server.AddDocument(3, "cat and dog live in the house"sv, DocumentStatus::kActual, { 5 });
    server.AddDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::kActual, { 4 });
    {
        auto found_docs = server.FindTopDocuments(
                "cat dog bird"sv,
                [](auto id, auto, auto) {
                    return id >= 3;
                });
        ASSERT_EQUAL(found_docs[0].id, 4);
        ASSERT_EQUAL(found_docs[1].id, 3);
    }
    {
        auto found_docs = server.FindTopDocuments(
                "cat dog bird"sv,
                [](auto, auto, auto rating) {
                    return rating == 5;
                });
        ASSERT_EQUAL(found_docs[0].id, 3);
        ASSERT_EQUAL(found_docs[1].id, 2);
    }
}

TEST(FindTopDocumentsWithSpecifiedStatus) {
    SearchServer server(""sv);
    server.AddDocument(1, "nobody lives in the house"sv, DocumentStatus::kIrrelevant, { 0 });
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::kBanned, { 5 });
    server.AddDocument(3, "cat and dog live in the house"sv, DocumentStatus::kRemoved, { 5 });
    server.AddDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::kActual, { 4 });
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::kIrrelevant);
        ASSERT_EQUAL_HINT(found_docs[0].id, 1, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::kBanned);
        ASSERT_EQUAL_HINT(found_docs[0].id, 2, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::kRemoved);
        ASSERT_EQUAL_HINT(found_docs[0].id, 3, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::kActual);
        ASSERT_EQUAL_HINT(found_docs[0].id, 4, "A relevant document with a specified status must be found"s);
    }
}

TEST(CorrectnessRelevance) {
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server("is are was a an in the with near at"sv);
    server.AddDocument(0, "a colorful parrot with green wings and red tail is lost"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(1, "a grey hound with black ears is found at the railway station"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(2, "a white cat with long furry tail is found near the red square"sv, DocumentStatus::kActual, ratings);
    {
        auto found_docs = server.FindTopDocuments("white cat long tail"sv);
        ASSERT_EQUAL(found_docs[0].id, 2);
        ASSERT(std::abs(found_docs[0].relevance - 0.462663) < ERROR_MARGIN);
        ASSERT_EQUAL(found_docs[1].id, 0);
        ASSERT(std::abs(found_docs[1].relevance - 0.0506831) < ERROR_MARGIN);
    }
}

TEST(RemoveDuplicates) {
    SearchServer server("and in with"sv);
    {
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        RemoveDuplicates(server);
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"sv, DocumentStatus::kActual, ratings);
    server.AddDocument(1, "black cat"sv, DocumentStatus::kActual, ratings);
    {
        ASSERT_EQUAL(server.GetDocumentCount(), 2);
        RemoveDuplicates(server);
        ASSERT_EQUAL(server.GetDocumentCount(), 2);
    }
    {
        server.AddDocument(2, "black cat"sv, DocumentStatus::kActual, ratings);
        RemoveDuplicates(server);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        const std::vector<int> answer = {0, 1};
        ASSERT_EQUAL(res, answer);
    }
    {
        server.AddDocument(2, "black cat"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(3, "cat black"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(4, "cat in black"sv, DocumentStatus::kActual, ratings);
        server.AddDocument(5, "black cat and black cat"sv, DocumentStatus::kActual, ratings);
        RemoveDuplicates(server);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        const std::vector<int> answer = {0, 1};
        ASSERT_EQUAL(res, answer);
    }
}
