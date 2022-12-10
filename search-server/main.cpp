#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

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

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_size = 1.0 / words.size();
        for (const auto& word : words) {
            word_to_document_freqs_[word][document_id] += inv_size;
        }
        document_count_ += 1;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);
        
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int document_count_ = 0;
    
    map<string, map<int, double>> word_to_document_freqs_;

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] != '-') {
                query.plus_words.insert(word);
            } else if (const string minus_word = word.substr(1); !IsStopWord(minus_word)) {
                query.minus_words.insert(minus_word);
            }
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> doc_to_relevance;
        for (const auto& plus_word : query.plus_words) {
            auto iter = word_to_document_freqs_.find(plus_word);
            if (iter != word_to_document_freqs_.end()) {
                const double idf = log((double)document_count_ / iter->second.size());
                for (const auto& [document_id, tf] : iter->second) {
                    doc_to_relevance[document_id] += tf * idf;
                }
            }
        }
        for (const auto& minus_word : query.minus_words) {
            auto iter = word_to_document_freqs_.find(minus_word);
            if (iter != word_to_document_freqs_.end()) {
                for (const auto [document_id, _] : iter->second) {
                    doc_to_relevance.erase(document_id);
                }
            }
        }
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : doc_to_relevance) {
            matched_documents.push_back({document_id, relevance});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}