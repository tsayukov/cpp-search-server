#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const double ERROR_MARGIN = 1e-6;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    cin >> ws;
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

template<typename Receiver, typename Source,
        typename Predicate, typename Inserter>
Receiver Filter(const Source& source,
                Predicate predicate, Inserter inserter,
                Receiver default_ctor = Receiver())
{
    Receiver receiver = default_ctor;
    for (const auto& elem : source) {
        if (predicate(elem)) {
            inserter(receiver, elem);
        }
    }
    return receiver;
}

constexpr auto SET_INSERTER = [](auto& receiver, const auto& elem) -> void {
    receiver.insert(elem);
};

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
            : id(id)
            , relevance(relevance)
            , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template<typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
            : stop_words_(Filter<set<string>>(
                    stop_words,
                    [](const auto& elem) {
                        if (elem.empty()) {
                            return false;
                        }
                        StringHasNotAnyForbiddenChars(elem);
                        return true;
                    },
                    SET_INSERTER)) {
    }

    explicit SearchServer(const string& stop_words)
            : SearchServer(SplitIntoWords(stop_words)) {
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    int GetDocumentId(int index) const {
        if (index < 0 || static_cast<size_t>(index) >= ids_.size()) {
            throw out_of_range(
                    "The passed index is out of range "s
                    "["s + to_string(0) + "; "s + to_string(GetDocumentCount()) + "). "s
                    "Given: " + to_string(index));
        }
        return ids_.at(index);
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                                   const vector<int>& ratings) {
        DocumentIdIsNotNegative(document_id);
        DocumentIdDoesntExist(document_id);
        const auto words = SplitIntoWordsNoStop(document);
        const double inv_size = 1.0 / words.size();
        for (const auto& word : words) {
            word_to_document_freqs_[word][document_id] += inv_size;
        }
        ids_.push_back(document_id);
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }

    template<typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query, Predicate predicate) const {
        auto result = FindAllDocuments(ParseQuery(raw_query), predicate);

        sort(result.begin(), result.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < ERROR_MARGIN) {
                     return lhs.rating > rhs.rating;
                 }
                 return lhs.relevance > rhs.relevance;
             });
        if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
            result.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return result;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus document_status) const {
        return FindTopDocuments(raw_query,
                                [document_status](int document_id, DocumentStatus status, int rating) {
                                    return status == document_status;
                                });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        DocumentIdIsNotNegative(document_id);
        const auto query = ParseQuery(raw_query);

        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            auto iter = word_to_document_freqs_.find(word);
            if (iter == word_to_document_freqs_.end()) {
                continue;
            }
            if (const auto& document_freqs = iter->second;
                document_freqs.count(document_id) > 0)
            {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            auto iter = word_to_document_freqs_.find(word);
            if (iter == word_to_document_freqs_.end()) {
                continue;
            }
            if (const auto& document_freqs = iter->second;
                document_freqs.count(document_id) > 0)
            {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> ids_;

    static void StringHasNotAnyForbiddenChars(const string& s) {
        if (any_of(s.begin(), s.end(),
                   [](const char c) {
                        return iscntrl(static_cast<unsigned char>(c));
                   }))
        {
            throw invalid_argument("Stop words contain forbidden characters from 0x00 to 0x1F"s);
        }
    }

    void DocumentIdIsNotNegative(int document_id) const {
        if (document_id < 0) {
            throw invalid_argument("The negative document id"s);
        }
    }

    void DocumentIdDoesntExist(int document_id) const {
        if (documents_.count(document_id) > 0) {
            throw invalid_argument("The passed document id already exists"s);
        }
    }

    [[nodiscard]] bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> result;
        const auto words = SplitIntoWords(text);
        for (const string& word : words) {
            StringHasNotAnyForbiddenChars(word);
            if (!IsStopWord(word)) {
                result.push_back(word);
            }
        }
        return result;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        const int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string content;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    QueryWord ParseQueryWord(string word) const {
        if (word[word.size() - 1] == '-') {
            throw invalid_argument("Incorrect form of minus words"s);
        }
        bool is_minus = word[0] == '-';
        if (is_minus) {
            if (word[1] == '-') {
                throw invalid_argument("Incorrect form of minus words"s);
            }
            word = word.substr(1);
        }
        return QueryWord{word, is_minus, IsStopWord(word)};
    }

    Query ParseQuery(const string& text) const {
        Query query;
        const auto words = SplitIntoWords(text);
        for (const string& word : words) {
            StringHasNotAnyForbiddenChars(word);
            const auto query_word = ParseQueryWord(word);
            if (!(query_word.is_stop)) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.content);
                } else {
                    query.plus_words.insert(query_word.content);
                }
            }
        }
        return query;
    }

    template<typename Predicate>
    vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const {
        map<int, double> doc_to_relevance;
        for (const auto& plus_word : query.plus_words) {
            auto iter = word_to_document_freqs_.find(plus_word);
            if (iter != word_to_document_freqs_.end()) {
                const auto& document_freqs = iter->second;
                const double idf = log(GetDocumentCount() * 1.0 / document_freqs.size());
                for (const auto& [document_id, tf] : document_freqs) {
                    const auto& document_data = documents_.at(document_id);
                    if (predicate(document_id, document_data.status, document_data.rating)) {
                        doc_to_relevance[document_id] += tf * idf;
                    }
                }
            }
        }
        for (const auto& minus_word : query.minus_words) {
            auto iter = word_to_document_freqs_.find(minus_word);
            if (iter != word_to_document_freqs_.end()) {
                const auto& document_freqs = iter->second;
                for (const auto [document_id, _] : document_freqs) {
                    doc_to_relevance.erase(document_id);
                }
            }
        }
        vector<Document> result;
        for (const auto [document_id, relevance] : doc_to_relevance) {
            result.emplace_back(document_id, relevance, documents_.at(document_id).rating);
        }
        return result;
    }
};

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}

// TEST FRAMEWORK

template <typename Container>
ostream& PrintContainer(ostream& os, const Container& container, const string& sep = ", "s) {
    auto it = begin(container);
    const auto end_it = end(container);
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

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
    os << "["s;
    PrintContainer(os, vec);
    return os << "]"s;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str_repr, const string& u_str_repr,
                     const string& file_name, const string& func_name, unsigned line_number, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file_name << "("s << line_number << "): "s << func_name << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str_repr << ", "s << u_str_repr << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& value_str_repr, const string& file_name, const string& func_name,
                unsigned line_number, const string& hint) {
    if (!value) {
        cerr << file_name << "("s << line_number << "): "s << func_name << ": "s;
        cerr << "ASSERT("s << value_str_repr << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(func, exception) \
    try {                             \
        func;                         \
        cerr << __FILE__ << "("s << __LINE__ << "): "s << __FUNCTION__ << ": "s; \
        cerr << "ASSERT("s << #func << ") failed. "s; \
        cerr << #exception << "must be thrown."s << endl; \
        abort();                      \
    } catch (const exception&) {      \
    } catch (...) {                   \
        cerr << __FILE__ << "("s << __LINE__ << "): "s << __FUNCTION__ << ": "s; \
        cerr << "ASSERT("s << #func << ") failed. "s; \
        cerr << "Incorrect exception. Expect "s << #exception << "."s << endl; \
        abort(); \
    }

template <typename Func>
void RunTestImpl(Func func, const string& func_name) {
    func();
    cerr << func_name << " OK"s << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

// UNIT TESTS

void TestConstructors() {
    ASSERT_THROW(SearchServer("in \x12the"s), invalid_argument);
    ASSERT_THROW(SearchServer(vector<string>{"in"s, "\x12the"s}), invalid_argument);
}

void TestGetDocumentId() {
    SearchServer server(""s);
    ASSERT_THROW(server.GetDocumentId(-1), out_of_range);
    ASSERT_THROW(server.GetDocumentId(0), out_of_range);

    const vector<int> ratings = {1, 2, 3};
    server.AddDocument(5, "nobody lives in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "cat lives in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(6, "cat and dog live in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(4, "cat and dog and bird live in the house"s, DocumentStatus::ACTUAL, ratings);
    ASSERT_THROW(server.GetDocumentId(-1), out_of_range);
    ASSERT_EQUAL(server.GetDocumentId(0), 5);
    ASSERT_EQUAL(server.GetDocumentId(2), 6);
    ASSERT_EQUAL(server.GetDocumentId(3), 4);
    ASSERT_THROW(server.GetDocumentId(4), out_of_range);
}

void TestAddDocument() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
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
                invalid_argument);
        auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.empty());
    }
}

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
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
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
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
        ASSERT_THROW(server.FindTopDocuments("cat -"s), invalid_argument);
        ASSERT_THROW(server.FindTopDocuments("cat --"s), invalid_argument);
        ASSERT_THROW(server.FindTopDocuments("cat -dog-"s), invalid_argument);
        ASSERT_THROW(server.FindTopDocuments("cat -d-o-g-"s), invalid_argument);
        ASSERT_THROW(server.FindTopDocuments("cat-"s), invalid_argument);
        ASSERT_THROW(server.FindTopDocuments("ca\x12t"s), invalid_argument);
    }
}

void TestMatchingDocuments() {
    const int doc_id = 42;
    const string content = "cats in the city of cats"s;
    const vector<int> ratings = {1, 2, 3};
    SearchServer server(""s);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    {
        auto [match_words, _] = server.MatchDocument("beautiful cats city"s, doc_id);
        ASSERT_EQUAL_HINT(match_words.size(), 2u,
                          "Matching words between a content of the document and the query without repetitions: "s
                                  "checks size of matched words"s);
        sort(match_words.begin(), match_words.end());
        const vector<string> answer = { "cats"s, "city"s };
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
    const vector<int> ratings = {1, 2, 3};
    SearchServer server(""s);
    server.AddDocument(5, "nobody lives in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "cat lives in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(6, "cat and dog live in the house"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(4, "cat and dog and bird live in the house"s, DocumentStatus::ACTUAL, ratings);
    auto found_docs = server.FindTopDocuments("cat dog bird"s);
    vector<double> result;
    for (const auto& document : found_docs) {
        result.push_back(document.relevance);
    }
    vector<double> answer = result;
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
    const vector<int> ratings = {1, 2, 3};
    SearchServer server("is are was a an in the with near at"s);
    server.AddDocument(0, "a colorful parrot with green wings and red tail is lost"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, "a grey hound with black ears is found at the railway station"s, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "a white cat with long furry tail is found near the red square"s, DocumentStatus::ACTUAL, ratings);
    {
        auto found_docs = server.FindTopDocuments("white cat long tail"s);
        ASSERT_EQUAL(found_docs[0].id, 2);
        ASSERT(abs(found_docs[0].relevance - 0.462663) < ERROR_MARGIN);
        ASSERT_EQUAL(found_docs[1].id, 0);
        ASSERT(abs(found_docs[1].relevance - 0.0506831) < ERROR_MARGIN);
    }
}

void TestSearchServer() {
    RUN_TEST(TestConstructors);
    RUN_TEST(TestGetDocumentId);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatchingDocuments);
    RUN_TEST(TestSortingDocumentsByRelevance);
    RUN_TEST(TestDocumentRating);
    RUN_TEST(TestFindTopDocumentsWithPredicate);
    RUN_TEST(TestFindTopDocumentsWithSpecifiedStatus);
    RUN_TEST(TestCorrectnessRelevance);
}

int main() {
    TestSearchServer();
    cout << "Search server testing finished"s << endl;
    return 0;
}