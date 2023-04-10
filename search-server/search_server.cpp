#include "search_server.h"

#include <numeric>
#include <stdexcept>

using namespace std::string_literals;

SearchServer::SearchServer(const std::string& stop_words)
        : SearchServer(SplitIntoWords(stop_words)) {
}

int SearchServer::GetDocumentCount() const noexcept {
    return documents_.size();
}

std::set<int>::const_iterator SearchServer::begin() const noexcept {
    return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const noexcept {
    return document_ids_.end();
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                               const std::vector<int>& ratings) {
    DocumentIdIsNotNegative(document_id);
    DocumentIdDoesntExist(document_id);

    document_ids_.insert(document_id);
    auto& document_data = documents_[document_id];
    document_data.rating = ComputeAverageRating(ratings);
    document_data.status = status;

    const auto words = SplitIntoWordsNoStop(document);
    const double inv_size = 1.0 / static_cast<double>(words.size());
    for (const auto& word : words) {
        word_to_document_frequencies_[word][document_id] += inv_size;
        document_data.word_frequencies[word] += inv_size;
    }
}

void SearchServer::RemoveDocument(int document_id) {
    if (auto doc_iter = documents_.find(document_id); doc_iter != documents_.end()) {
        const auto& document_data = doc_iter->second;
        for (const auto& [word, _] : document_data.word_frequencies) {
            auto word_iter = word_to_document_frequencies_.find(word);
            auto& documents_with_that_word = word_iter->second;
            documents_with_that_word.erase(document_id);

            if (documents_with_that_word.empty()) {
                word_to_document_frequencies_.erase(word_iter);
            }
        }

        documents_.erase(doc_iter);
        document_ids_.erase(document_id);
    }
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
                                                     DocumentStatus document_status) const {
    return FindTopDocuments(raw_query,
                            [document_status](int /*document_id*/, DocumentStatus status, int /*rating*/) {
                                return status == document_status;
                            });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::tuple<std::vector<std::string>, DocumentStatus>
SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    DocumentIdIsNotNegative(document_id);
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const auto& word : query.plus_words) {
        auto iter = word_to_document_frequencies_.find(word);
        if (iter == word_to_document_frequencies_.end()) {
            continue;
        }
        if (const auto& document_frequencies = iter->second; document_frequencies.count(document_id) > 0) {
            matched_words.push_back(word);
        }
    }
    for (const auto& word : query.minus_words) {
        auto iter = word_to_document_frequencies_.find(word);
        if (iter == word_to_document_frequencies_.end()) {
            continue;
        }
        if (const auto& document_frequencies = iter->second; document_frequencies.count(document_id) > 0) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string, double> empty_map;

    if (auto it = documents_.find(document_id); it != documents_.end()) {
        return it->second.word_frequencies;
    }
    return empty_map;
}

void SearchServer::StringHasNotAnyForbiddenChars(const std::string& s) {
    if (any_of(s.begin(), s.end(),
               [](const char c) {
                   return iscntrl(static_cast<unsigned char>(c));
               })) {
        throw std::invalid_argument("Stop words contain forbidden characters from 0x00 to 0x1F"s);
    }
}

void SearchServer::DocumentIdIsNotNegative(int document_id) {
    if (document_id < 0) {
        throw std::invalid_argument("The negative document id"s);
    }
}

void SearchServer::DocumentIdDoesntExist(int document_id) const {
    if (documents_.count(document_id) > 0) {
        throw std::invalid_argument("The passed document id already exists"s);
    }
}

[[nodiscard]] bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> result;
    const auto words = SplitIntoWords(text);
    for (const auto& word : words) {
        StringHasNotAnyForbiddenChars(word);
        if (!IsStopWord(word)) {
            result.push_back(word);
        }
    }
    return result;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    const int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string word) const {
    bool is_minus = word[0] == '-';
    if (is_minus) {
        if (word.size() == 1 || word[1] == '-') {
            throw std::invalid_argument("Incorrect form of minus words"s);
        }
        word = word.substr(1);
    }
    return QueryWord{word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
    const auto words = SplitIntoWords(text);
    for (const auto& word : words) {
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