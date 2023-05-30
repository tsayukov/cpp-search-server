#pragma once

#include "unit_test_tools.h"
#include "remove_duplicates.h"
#include "search_server.h"
#include "paginator.h"
#include "flatten_container.h"

#include <forward_list>
#include <list>

namespace unit_tests {

using namespace unit_test_tools;
using namespace std::string_literals;
using namespace std::string_view_literals;

inline constexpr double ERROR_MARGIN = 1e-6;

inline void TestConstructors() {
    ASSERT_THROW(SearchServer("in \x12the"sv), std::invalid_argument);
    ASSERT_THROW(SearchServer(std::vector<std::string>{"in"s, "\x12the"s}), std::invalid_argument);
    ASSERT_THROW(SearchServer(std::vector<std::string_view>{"in"sv, "\x12the"sv}), std::invalid_argument);
}

inline void TestRangeBasedForLoop() {
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
        server.AddDocument(0, "white cat"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(1, "black cat"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(5, "blue cat"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(10, "another blue cat"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(100, "blue cat and blue kitty"sv, DocumentStatus::ACTUAL, ratings);
        std::vector<int> res;
        for (const auto id : server) {
            res.push_back(id);
        }
        std::sort(res.begin(), res.end());
        std::vector<int> answer = {0, 1, 5, 10, 100};
        ASSERT_EQUAL(res, answer);
    }
}

inline void TestAddDocument() {
    const int doc_id = 42;
    const auto content = "cat in the city"sv;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server(""sv);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("in"sv);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server(""sv);
        ASSERT_THROW(
                server.AddDocument(doc_id, "cat in \x12the city"sv, DocumentStatus::ACTUAL, ratings),
                std::invalid_argument);
        auto found_docs = server.FindTopDocuments("in"sv);
        ASSERT(found_docs.empty());
    }
}

inline void TestRemoveDocument() {
    SearchServer server("and in with"sv);
    {
        server.RemoveDocument(1);
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        server.RemoveDocument(100);
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "black cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(5, "blue cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(10, "another blue cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(100, "blue cat and blue kitty"sv, DocumentStatus::ACTUAL, ratings);
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

inline void TestGetWordFrequencies() {
    SearchServer server("and in with"sv);
    {
        ASSERT(server.GetWordFrequencies(1).empty());
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "black cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(5, "blue cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(10, "another blue cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(100, "blue cat and blue kitty"sv, DocumentStatus::ACTUAL, ratings);
    {
        std::map<std::string_view, double> answer = {{"white"sv, 1.0 / 2}, {"cat"sv, 1.0 / 2}};
        ASSERT_EQUAL(server.GetWordFrequencies(0), answer);
    }
    {
        std::map<std::string_view, double> answer = {{"blue"sv, 2.0 / 4}, {"cat"sv, 1.0 / 4}, {"kitty"sv, 1.0 / 4}};
        ASSERT_EQUAL(server.GetWordFrequencies(100), answer);
    }
}

inline void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const auto content = "cat in the city"sv;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server("in the"sv);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("in"sv);
        ASSERT_HINT(found_docs.empty(), "Stop words must be excluded from documents"s);
    }
}

inline void TestExcludeDocumentsWithMinusWords() {
    const int doc_id = 42;
    const auto content = "cat in the city"sv;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
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

inline void TestMatchingDocuments() {
    const int doc_id = 42;
    const auto content = "cats in the city of cats"sv;
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
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

inline void TestSortingDocumentsByRelevance() {
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server(""sv);
    server.AddDocument(5, "nobody lives in the house"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(6, "cat and dog live in the house"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::ACTUAL, ratings);
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

inline void TestDocumentRating() {
    SearchServer server(""sv);
    server.AddDocument(5, "nobody lives in the house"sv, DocumentStatus::ACTUAL, {});
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::ACTUAL, {5});
    server.AddDocument(3, "dog lives in the house"sv, DocumentStatus::ACTUAL, {5, 5, 5});
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

inline void TestFindTopDocumentsWithPredicate() {
    SearchServer server(""sv);
    server.AddDocument(1, "nobody lives in the house"sv, DocumentStatus::IRRELEVANT, { 0 });
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::ACTUAL, { 5 });
    server.AddDocument(3, "cat and dog live in the house"sv, DocumentStatus::ACTUAL, { 5 });
    server.AddDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::ACTUAL, { 4 });
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

inline void TestFindTopDocumentsWithSpecifiedStatus() {
    SearchServer server(""sv);
    server.AddDocument(1, "nobody lives in the house"sv, DocumentStatus::IRRELEVANT, { 0 });
    server.AddDocument(2, "cat lives in the house"sv, DocumentStatus::BANNED, { 5 });
    server.AddDocument(3, "cat and dog live in the house"sv, DocumentStatus::REMOVED, { 5 });
    server.AddDocument(4, "cat and dog and bird live in the house"sv, DocumentStatus::ACTUAL, { 4 });
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL_HINT(found_docs[0].id, 1, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::BANNED);
        ASSERT_EQUAL_HINT(found_docs[0].id, 2, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::REMOVED);
        ASSERT_EQUAL_HINT(found_docs[0].id, 3, "A relevant document with a specified status must be found"s);
    }
    {
        auto found_docs = server.FindTopDocuments("house"sv,DocumentStatus::ACTUAL);
        ASSERT_EQUAL_HINT(found_docs[0].id, 4, "A relevant document with a specified status must be found"s);
    }
}

inline void TestCorrectnessRelevance() {
    const std::vector<int> ratings = {1, 2, 3};
    SearchServer server("is are was a an in the with near at"sv);
    server.AddDocument(0, "a colorful parrot with green wings and red tail is lost"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "a grey hound with black ears is found at the railway station"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "a white cat with long furry tail is found near the red square"sv, DocumentStatus::ACTUAL, ratings);
    {
        auto found_docs = server.FindTopDocuments("white cat long tail"sv);
        ASSERT_EQUAL(found_docs[0].id, 2);
        ASSERT(std::abs(found_docs[0].relevance - 0.462663) < ERROR_MARGIN);
        ASSERT_EQUAL(found_docs[1].id, 0);
        ASSERT(std::abs(found_docs[1].relevance - 0.0506831) < ERROR_MARGIN);
    }
}

inline void TestRemoveDuplicates() {
    SearchServer server("and in with"sv);
    {
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        RemoveDuplicates(server);
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
    }
    const std::vector<int> ratings = {1, 2, 3};
    server.AddDocument(0, "white cat"sv, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "black cat"sv, DocumentStatus::ACTUAL, ratings);
    {
        ASSERT_EQUAL(server.GetDocumentCount(), 2);
        RemoveDuplicates(server);
        ASSERT_EQUAL(server.GetDocumentCount(), 2);
    }
    {
        server.AddDocument(2, "black cat"sv, DocumentStatus::ACTUAL, ratings);
        RemoveDuplicates(server);
        std::vector<int> res(server.begin(), server.end());
        std::sort(res.begin(), res.end());
        const std::vector<int> answer = {0, 1};
        ASSERT_EQUAL(res, answer);
    }
    {
        server.AddDocument(2, "black cat"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(3, "cat black"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(4, "cat in black"sv, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(5, "black cat and black cat"sv, DocumentStatus::ACTUAL, ratings);
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

inline void TestPaginator() {
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

namespace flatten_container_tests {

template<typename T>
using RAContainer = std::vector<T>;

template<typename T>
using BContainer = std::list<T>;

template<typename T>
using FContainer = std::forward_list<T>;

void TestIteratorCategory() {
    using std::is_same_v;
    using AnyType = int;

    ASSERT((is_same_v<FlattenContainer<RAContainer<RAContainer<AnyType>>>::iterator::iterator_category,
                      std::bidirectional_iterator_tag>));
    ASSERT((is_same_v<FlattenContainer<BContainer<RAContainer<AnyType>>>::iterator::iterator_category,
                      std::bidirectional_iterator_tag>));
    ASSERT((is_same_v<FlattenContainer<FContainer<RAContainer<AnyType>>>::iterator::iterator_category,
                      std::forward_iterator_tag>));

    ASSERT((is_same_v<FlattenContainer<RAContainer<BContainer<AnyType>>>::iterator::iterator_category,
                      std::bidirectional_iterator_tag>));
    ASSERT((is_same_v<FlattenContainer<BContainer<BContainer<AnyType>>>::iterator::iterator_category,
                      std::bidirectional_iterator_tag>));
    ASSERT((is_same_v<FlattenContainer<FContainer<BContainer<AnyType>>>::iterator::iterator_category,
                      std::forward_iterator_tag>));

    ASSERT((is_same_v<FlattenContainer<RAContainer<FContainer<AnyType>>>::iterator::iterator_category,
                      std::forward_iterator_tag>));
    ASSERT((is_same_v<FlattenContainer<BContainer<FContainer<AnyType>>>::iterator::iterator_category,
                      std::forward_iterator_tag>));
    ASSERT((is_same_v<FlattenContainer<FContainer<FContainer<AnyType>>>::iterator::iterator_category,
                      std::forward_iterator_tag>));
}

std::vector<std::size_t> GenerateBottomSizes(std::size_t top_size,
                                             std::size_t min_bottom_size, std::size_t max_bottom_size) {
    std::vector<std::size_t> bottom_sizes(top_size);
    std::generate(
            bottom_sizes.begin(), bottom_sizes.end(),
            [min_bottom_size, max_bottom_size] {
                return Generator<std::size_t>::Get(min_bottom_size, max_bottom_size);
            });
    return bottom_sizes;
}

template<typename T>
std::vector<T> GenerateAnswer(const std::vector<std::size_t>& bottom_sizes) {
    std::vector<T> answer;
    const std::size_t answer_size = std::accumulate(bottom_sizes.cbegin(), bottom_sizes.cend(), std::size_t(0));
    answer.reserve(answer_size);
    for (std::size_t i = 0; i < answer_size; ++i) {
        answer.push_back(Generator<T>::Get());
    }
    return answer;
}

template<typename TopContainer, typename ValueType = typename TopContainer::value_type::value_type>
TopContainer GetTopContainer(const std::vector<ValueType>& answer,
                             const std::vector<std::size_t>& bottom_sizes) {
    using BottomContainer = typename TopContainer::value_type;

    TopContainer top_container(bottom_sizes.size(), BottomContainer{});
    auto top_iter = top_container.begin();
    for (std::size_t i = 0, j = 0; i < bottom_sizes.size(); ++i, ++top_iter) {
        *top_iter = BottomContainer(bottom_sizes[i], ValueType{});
        std::size_t bottom_size = bottom_sizes[i];
        for (auto bottom_iter = top_iter->begin(); bottom_size != 0; ++bottom_iter, --bottom_size, ++j) {
            *bottom_iter = answer[j];
        }
    }
    return top_container;
}

template<typename TopContainer, typename ValueType = typename TopContainer::value_type::value_type>
std::vector<ValueType> GetResultByRangeBasedForLoop(TopContainer& top_container, std::size_t answer_size) {
    std::vector<ValueType> result;
    result.reserve(answer_size);
    const auto flatten_container = MakeFlattenContainer(std::move(top_container));
    for (const auto& element : flatten_container) {
        result.push_back(element);
    }
    return result;
}

template<typename TopContainer>
void TestRangeBasedFor(std::size_t top_size,
                       std::size_t min_bottom_size, std::size_t max_bottom_size) {
    using ValueType = typename TopContainer::value_type::value_type;

    auto bottom_sizes = GenerateBottomSizes(top_size, min_bottom_size, max_bottom_size);
    auto answer = GenerateAnswer<ValueType>(bottom_sizes);
    auto top_container = GetTopContainer<TopContainer>(answer, bottom_sizes);
    auto result = GetResultByRangeBasedForLoop(top_container, answer.size());
    ASSERT_EQUAL(result, answer);
}

template<typename T, std::size_t TopSize, std::size_t MinBottomSize, std::size_t MaxBottomSize>
void AllTestsRangeBasedFor() {
    TestRangeBasedFor<RAContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestRangeBasedFor<BContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestRangeBasedFor<FContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);

    TestRangeBasedFor<RAContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestRangeBasedFor<BContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestRangeBasedFor<FContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);

    TestRangeBasedFor<RAContainer<FContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestRangeBasedFor<BContainer<FContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestRangeBasedFor<FContainer<FContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
}

template<typename TopContainer, typename ValueType = typename TopContainer::value_type::value_type>
std::vector<ValueType> GetResultByReverseOrder(TopContainer& top_container, std::size_t answer_size) {
    std::vector<ValueType> result;
    result.reserve(answer_size);
    const auto flatten_container = MakeFlattenContainer(std::move(top_container));
    auto begin = flatten_container.cbegin();
    auto end = flatten_container.cend();
    if (begin != end) {
        do {
            --end;
            result.push_back(*end);
        } while (begin != end);
    }
    return result;
}

template<typename TopContainer>
void TestReverseOrder(std::size_t top_size,
                      std::size_t min_bottom_size, std::size_t max_bottom_size) {
    using ValueType = typename TopContainer::value_type::value_type;

    auto bottom_sizes = GenerateBottomSizes(top_size, min_bottom_size, max_bottom_size);
    auto answer = GenerateAnswer<ValueType>(bottom_sizes);
    auto top_container = GetTopContainer<TopContainer>(answer, bottom_sizes);
    auto result = GetResultByReverseOrder(top_container, answer.size());
    std::reverse(answer.begin(), answer.end());
    ASSERT_EQUAL(result, answer);
}

template<typename T, std::size_t TopSize, std::size_t MinBottomSize, std::size_t MaxBottomSize>
void AllTestsReverseOrder() {
    TestReverseOrder<RAContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestReverseOrder<BContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);

    TestReverseOrder<RAContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestReverseOrder<BContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
}

template<typename TopContainer, typename ValueType = typename TopContainer::value_type::value_type>
std::vector<ValueType> GetResultByMutateValue(TopContainer& top_container, std::size_t answer_size) {
    std::vector<ValueType> result;
    result.reserve(answer_size);
    auto flatten_container = MakeFlattenContainer(std::move(top_container));
    for (auto& elem : flatten_container) {
        elem = ValueType{};
    }
    for (const auto& element : flatten_container) {
        result.push_back(element);
    }
    return result;
}

template<typename TopContainer>
void TestMutateValue(std::size_t top_size,
                       std::size_t min_bottom_size, std::size_t max_bottom_size) {
    using ValueType = typename TopContainer::value_type::value_type;

    auto bottom_sizes = GenerateBottomSizes(top_size, min_bottom_size, max_bottom_size);
    auto answer = GenerateAnswer<ValueType>(bottom_sizes);
    auto top_container = GetTopContainer<TopContainer>(answer, bottom_sizes);
    auto result = GetResultByMutateValue(top_container, answer.size());
    std::for_each(answer.begin(), answer.end(), [](auto& element) { return element = ValueType{}; });
    ASSERT_EQUAL(result, answer);
}

template<typename T, std::size_t TopSize, std::size_t MinBottomSize, std::size_t MaxBottomSize>
void AllTestsMutateValue() {
    TestMutateValue<RAContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestMutateValue<BContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestMutateValue<FContainer<RAContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);

    TestMutateValue<RAContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestMutateValue<BContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestMutateValue<FContainer<BContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);

    TestMutateValue<RAContainer<FContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestMutateValue<BContainer<FContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
    TestMutateValue<FContainer<FContainer<T>>>(TopSize, MinBottomSize, MaxBottomSize);
}

} // namespace flatten_container_tests

void RunAllTestsFlattenContainer() {
    using namespace flatten_container_tests;

    RUN_TEST(TestIteratorCategory);

    RUN_TEST((AllTestsRangeBasedFor<int, 0, 0, 0>));
    RUN_TEST((AllTestsRangeBasedFor<int, 10, 0, 0>));
    RUN_TEST((AllTestsRangeBasedFor<int, 10, 0, 1>));
    RUN_TEST((AllTestsRangeBasedFor<int, 10, 2, 3>));

    RUN_TEST((AllTestsReverseOrder<int, 0, 0, 0>));
    RUN_TEST((AllTestsReverseOrder<int, 10, 0, 0>));
    RUN_TEST((AllTestsReverseOrder<int, 10, 0, 1>));
    RUN_TEST((AllTestsReverseOrder<int, 10, 2, 3>));

    RUN_TEST((AllTestsMutateValue<int, 0, 0, 0>));
    RUN_TEST((AllTestsMutateValue<int, 10, 0, 0>));
    RUN_TEST((AllTestsMutateValue<int, 10, 0, 1>));
    RUN_TEST((AllTestsMutateValue<int, 10, 2, 3>));
}

} // namespace unit_tests

inline void RunAllTests() {
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
    RUN_TEST(RunAllTestsFlattenContainer);
}