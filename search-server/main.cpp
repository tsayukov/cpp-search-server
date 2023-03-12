#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "paginator.h"
#include "remove_duplicates.h"
#include "request_queue.h"
#include "search_server.h"
#include "test_example_functions.h"

using namespace std;
using namespace std::string_literals;

// UNIT TEST FRAMEWORK

namespace {
template <typename Container>
std::ostream& PrintContainer(std::ostream& os, const Container& container, const std::string& sep = ", "s);

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vector) {
    os << "["s;
    PrintContainer(os, vector);
    return os << "]"s;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& set) {
    os << "{"s;
    PrintContainer(os, set);
    return os << "}"s;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& map) {
    os << "{"s;
    PrintContainer(os, map);
    return os << "}"s;
}

template <typename F, typename S>
std::ostream& operator<<(std::ostream& os, const std::pair<F, S>& pair) {
    return os << pair.first << ": "s << pair.second;
}

template <typename Container>
std::ostream& PrintContainer(std::ostream& os, const Container& container, const std::string& sep) {
    auto it = std::begin(container);
    const auto end_it = std::end(container);
    if (it == end_it) {
        return os;
    }
    os << *it;
    ++it;
    for (; it != end_it; ++it) {
        os << sep << *it;
    }
    return os;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str_repr, const std::string& u_str_repr,
                     const std::string& file_name, const std::string& func_name, unsigned line_number,
                     const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file_name << "("s << line_number << "): "s << func_name << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str_repr << ", "s << u_str_repr << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& value_str_repr,
                const std::string& file_name, const std::string& func_name, unsigned line_number,
                const std::string& hint) {
    if (!value) {
        std::cerr << file_name << "("s << line_number << "): "s << func_name << ": "s;
        std::cerr << "ASSERT("s << value_str_repr << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

#define ASSERT(expr) AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(func, exception)                                                    \
    try {                                                                                \
        func;                                                                            \
        std::cerr << __FILE__ << "("s << __LINE__ << "): "s << __FUNCTION__ << ": "s;    \
        std::cerr << "ASSERT("s << #func << ") failed. "s;                               \
        std::cerr << #exception << "must be thrown."s << std::endl;                      \
        abort();                                                                         \
    } catch (const exception&) {                                                         \
    } catch (...) {                                                                      \
        std::cerr << __FILE__ << "("s << __LINE__ << "): "s << __FUNCTION__ << ": "s;    \
        std::cerr << "ASSERT("s << #func << ") failed. "s;                               \
        std::cerr << "Incorrect exception. Expect "s << #exception << "."s << std::endl; \
        std::abort();                                                                    \
    }

template <typename Func>
void RunTestImpl(Func func, const std::string& func_name) {
    func();
    std::cerr << func_name << " OK"s << std::endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)
} // the end of the anonymous namespace

// UNIT TESTS

namespace unit_tests {
void TestConstructors() {
    ASSERT_THROW(SearchServer("in \x12the"s), std::invalid_argument);
    ASSERT_THROW(SearchServer(std::vector<std::string>{"in"s, "\x12the"s}), std::invalid_argument);
}

void TestRangeBasedForLoop() {
    {
        const SearchServer empty_server(""s);
        std::vector<int> res;
        for (const auto id : empty_server) {
            res.push_back(id);
        }
        ASSERT(res.empty());
    }
    {
        const std::vector<int> ratings = {1, 2, 3};
        SearchServer server("and in with"s);
        server.AddDocument(0, "white cat"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(1, "black cat"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(5, "blue cat"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(10, "another blue cat"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(100, "blue cat and blue kitty"s, DocumentStatus::ACTUAL, ratings);
        std::vector<int> res;
        for (const auto id : server) {
            res.push_back(id);
        }
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 1, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
}

void TestAddDocument() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server(""s);
        ASSERT_THROW(
                server.AddDocument(doc_id, "cat in \x12the city"s, DocumentStatus::ACTUAL, ratings),
                std::invalid_argument);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.empty());
    }
}

void TestRemoveDocument() {
    SearchServer server("and in with"s);
    {
        server.RemoveDocument(1);
        ASSERT_EQUAL(server.GetDocumentCount(), 0u);
        server.RemoveDocument(100);
        ASSERT_EQUAL(server.GetDocumentCount(), 0u);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "black cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(5, "blue cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(10, "another blue cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(100, "blue cat and blue kitty"s, DocumentStatus::ACTUAL, ratings);
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

void TestGetWordFrequencies() {
    SearchServer server("and in with"s);
    {
        ASSERT(server.GetWordFrequencies(1).empty());
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "black cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(5, "blue cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(10, "another blue cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(100, "blue cat and blue kitty"s, DocumentStatus::ACTUAL, ratings);
    {
        std::map<std::string, double> answer = {{"white"s, 1.0 / 2}, {"cat"s, 1.0 / 2}};
        ASSERT_EQUAL(server.GetWordFrequencies(0), answer);
    }
    {
        std::map<std::string, double> answer = {{"blue"s, 2.0 / 4}, {"cat"s, 1.0 / 4}, {"kitty"s, 1.0 / 4}};
        ASSERT_EQUAL(server.GetWordFrequencies(100), answer);
    }
}

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_HINT(found_docs.empty(),
                    "Stop words must be excluded from documents"s);
    }
}

void TestExcludeDocumentsWithMinusWords() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""s);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    {
        auto found_docs = server.FindTopDocuments("cat -city -town -village"s);
        ASSERT_HINT(found_docs.empty(), "Relevant documents with minus words must be excluded from the result"s);

        found_docs = server.FindTopDocuments("cat -dog"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u,
                          "Relevant documents without minus words must be found"s);
    }
    {
        ASSERT_THROW(server.FindTopDocuments("cat -"s), std::invalid_argument);
        ASSERT_THROW(server.FindTopDocuments("cat --"s), std::invalid_argument);
        server.FindTopDocuments("cat-"s);
        server.FindTopDocuments("cat -dog-"s);
        server.FindTopDocuments("cat -d-o-g"s);
        ASSERT_THROW(server.FindTopDocuments("ca\x12t"s), std::invalid_argument);
    }
}

void TestMatchingDocuments() {
    const int doc_id = 42;
    const std::string content = "cats in the city of cats"s;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""s);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    {
        auto [match_words, _] = server.MatchDocument("beautiful cats city"s, doc_id);
        ASSERT_EQUAL_HINT(match_words.size(), 2u,
                          "Matching words between a content of the document and the query without repetitions: "s
                                  "checks size of matched words"s);
        sort(match_words.begin(), match_words.end());
        const std::vector<std::string> answer = { "cats"s, "city"s };
        ASSERT_EQUAL_HINT(match_words, answer,
                          "Matching words between a content of the document and the query without repetitions: "s
                                  "checks matched words"s);
    }
    {
        const auto [match_words, _] = server.MatchDocument("cats -city"s, doc_id);
        ASSERT_HINT(match_words.empty(), "Minus words must be excluded from matching"s);
    }
}

void TestSortingDocumentsByRelevance() {
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""s);
    server.AddDocument(5, "nobody lives in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "cat lives in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(6, "cat and dog live in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(4, "cat and dog and bird live in the house"s, DocumentStatus::ACTUAL, ratings);
    auto found_docs = server.FindTopDocuments("cat dog bird"s);
    std::vector<double> result;
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

void TestDocumentRating() {
    SearchServer server(""s);
    server.AddDocument(5, "nobody lives in the house"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(2, "cat lives in the house"s, DocumentStatus::ACTUAL, {5});
    server.AddDocument(3, "dog lives in the house"s, DocumentStatus::ACTUAL, {5, 5, 5});
    {
        auto found_docs = server.FindTopDocuments("nobody"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 0,
                          "Nobody rated the document, so the rating must be equal to zero"s);
    }
    {
        auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 5, "One rating"s);
    }
    {
        auto found_docs = server.FindTopDocuments("dog"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, (5 + 5 + 5) / 3, "A few ratings"s);
    }
}

void TestFindTopDocumentsWithPredicate() {
    SearchServer server(""s);
    server.AddDocument(1, "nobody lives in the house"s, DocumentStatus::IRRELEVANT, { 0 });
    server.AddDocument(2, "cat lives in the house"s, DocumentStatus::ACTUAL, { 5 });
    server.AddDocument(3, "cat and dog live in the house"s, DocumentStatus::ACTUAL, { 5 });
    server.AddDocument(4, "cat and dog and bird live in the house"s, DocumentStatus::ACTUAL, { 4 });
    {
        auto found_docs = server.FindTopDocuments(
                "cat dog bird"s,
                [](auto id, auto status, auto rating) {
                    return id >= 3;
                });
        ASSERT_EQUAL(found_docs[0].id, 4);
        ASSERT_EQUAL(found_docs[1].id, 3);
    }
    {
        auto found_docs = server.FindTopDocuments(
                "cat dog bird"s,
                [](auto id, auto status, auto rating) {
                    return rating == 5;
                });
        ASSERT_EQUAL(found_docs[0].id, 3);
        ASSERT_EQUAL(found_docs[1].id, 2);
    }
}

void TestFindTopDocumentsWithSpecifiedStatus() {
    SearchServer server(""s);
    server.AddDocument(1, "nobody lives in the house"s, DocumentStatus::IRRELEVANT, { 0 });
    server.AddDocument(2, "cat lives in the house"s, DocumentStatus::BANNED, { 5 });
    server.AddDocument(3, "cat and dog live in the house"s, DocumentStatus::REMOVED, { 5 });
    server.AddDocument(4, "cat and dog and bird live in the house"s, DocumentStatus::ACTUAL, { 4 });
    {
        auto found_docs = server.FindTopDocuments("house"s,DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL_HINT(found_docs[0].id, 1, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"s,DocumentStatus::BANNED);
        ASSERT_EQUAL_HINT(found_docs[0].id, 2, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"s,DocumentStatus::REMOVED);
        ASSERT_EQUAL_HINT(found_docs[0].id, 3, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"s,DocumentStatus::ACTUAL);
        ASSERT_EQUAL_HINT(found_docs[0].id, 4, "A relevant document with a specified status must be found"s);
    }
}

void TestCorrectnessRelevance() {
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server("is are was a an in the with near at"s);
    server.AddDocument(0, "a colorful parrot with green wings and red tail is lost"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "a grey hound with black ears is found at the railway station"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "a white cat with long furry tail is found near the red square"s, DocumentStatus::ACTUAL, ratings);
    {
        auto found_docs = server.FindTopDocuments("white cat long tail"s);
        ASSERT_EQUAL(found_docs[0].id, 2);
        ASSERT(std::abs(found_docs[0].relevance - 0.462663) < ERROR_MARGIN);
        ASSERT_EQUAL(found_docs[1].id, 0);
        ASSERT(std::abs(found_docs[1].relevance - 0.0506831) < ERROR_MARGIN);
    }
}

void TestRemoveDuplicates() {
    SearchServer server("and in with"s);
    {
        ASSERT_EQUAL(server.GetDocumentCount(), 0u);
        RemoveDuplicates(server);
        ASSERT_EQUAL(server.GetDocumentCount(), 0u);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "black cat"s, DocumentStatus::ACTUAL, ratings);
    {
        ASSERT_EQUAL(server.GetDocumentCount(), 2u);
        RemoveDuplicates(server);
        ASSERT_EQUAL(server.GetDocumentCount(), 2u);
    }
    {
        server.AddDocument(2, "black cat"s, DocumentStatus::ACTUAL, ratings);
        RemoveDuplicates(server);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        const std::vector<int> answer = {0, 1};
        ASSERT_EQUAL(res, answer);
    }
    {
        server.AddDocument(2, "black cat"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(3, "cat black"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(4, "cat in black"s, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(5, "black cat and black cat"s, DocumentStatus::ACTUAL, ratings);
        RemoveDuplicates(server);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        const std::vector<int> answer = {0, 1};
        ASSERT_EQUAL(res, answer);
    }
}

template<typename T>
std::vector<std::vector<T>> PaginateIntoVectors(const std::vector<T>& source, const size_t page_size) {
    std::vector<std::vector<T>> paged_vector;
    auto pages = Paginate(source, page_size);
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        paged_vector.emplace_back(page.begin(), page.end());
    }
    return paged_vector;
}

void TestPaginator() {
    {
        const std::vector<int> empty_vector;
        const std::vector<std::vector<int>> answer = {};
        ASSERT_EQUAL(PaginateIntoVectors(empty_vector, 10), answer);
    }
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    {
        const std::vector<std::vector<int>> answer = {};
        ASSERT_EQUAL(PaginateIntoVectors(vec, 0), answer);
    }
    {
        const std::vector<std::vector<int>> answer = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};
        ASSERT_EQUAL(PaginateIntoVectors(vec, 10), answer);
    }
    {
        const std::vector<std::vector<int>> answer = {{1, 2, 3, 4, 5, 6, 7, 8}, {9, 10}};
        ASSERT_EQUAL(PaginateIntoVectors(vec, 8), answer);
    }
    {
        const std::vector<std::vector<int>> answer = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};
        ASSERT_EQUAL(PaginateIntoVectors(vec, 1'000'000'000), answer);
    }
}

void AddManyRandomDocuments(SearchServer& server, unsigned int documents, unsigned int diversity_ratio) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<std::string> pets = {"dog"s, "goat"s, "pig"s, "sheep"s, "cat"s, "chicken"s, "duck"s};
    std::uniform_int_distribution<std::size_t> pet_distribution(0, pets.size() - 1);

    std::uniform_int_distribution<unsigned int> name_distribution(0, diversity_ratio);

    const std::vector<int> ratings = {1, 2, 3};
    for (unsigned int id = 0; id < documents; ++id) {
        const std::string document = pets[pet_distribution(gen)]
                                   + " "s
                                   + std::to_string(name_distribution(gen))
                                   + " in the "s
                                   + std::to_string(name_distribution(gen))
                                   + "th street"s;
        server.AddDocument(static_cast<int>(id), document, DocumentStatus::ACTUAL, ratings);
    }
}

void ProfilingSearchServer() {
    SearchServer server("and in with"s);
    AddManyRandomDocuments(server, 100'000, 1'000);
    std::cerr << GetWordFrequencies(server, 50'000) << std::endl;
    RemoveDocument(server, 50'000);
}

void ProfilingRemoveDuplicates() {
    SearchServer server("and in with"s);
    AddManyRandomDocuments(server, 1'000, 10);
    RemoveDuplicatesWithProfiling(server);
}
} // the end of the namespace `unit_tests`

void TestSearchServer() {
    using namespace unit_tests;

    RUN_TEST(TestConstructors);
    RUN_TEST(TestRangeBasedForLoop);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestRemoveDocument);
    RUN_TEST(TestGetWordFrequencies);
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatchingDocuments);
    RUN_TEST(TestSortingDocumentsByRelevance);
    RUN_TEST(TestDocumentRating);
    RUN_TEST(TestFindTopDocumentsWithPredicate);
    RUN_TEST(TestFindTopDocumentsWithSpecifiedStatus);
    RUN_TEST(TestCorrectnessRelevance);
    RUN_TEST(TestRemoveDuplicates);
    RUN_TEST(TestPaginator);

    RUN_TEST(ProfilingSearchServer);
    RUN_TEST(ProfilingRemoveDuplicates);
}

int main() {
    TestSearchServer();
    std::cout << "Search server testing finished"s << std::endl;
    return 0;
}