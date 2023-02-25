#include "search_server.h"

#include <numeric>
#include <stdexcept>

using std::operator""s;

SearchServer::SearchServer(const std::string& stop_words)
        : SearchServer(SplitIntoWords(stop_words)) {
}

int SearchServer::GetDocumentCount() const noexcept {
    return documents_.size();
}

int SearchServer::GetDocumentId(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= ids_.size()) {
        throw std::out_of_range(
                "The passed index is out of range "s
                "["s + std::to_string(0) + "; "s + std::to_string(GetDocumentCount()) + "). "s
                "Given: "s + std::to_string(index));
    }

    return ids_.at(index);
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) {
    DocumentIdIsNotNegative(document_id);
    DocumentIdDoesntExist(document_id);

    const auto words = SplitIntoWordsNoStop(document);
    const double inv_size = 1.0 / static_cast<double>(words.size());
    for (const auto& word : words) {
        word_to_document_freqs_[word][document_id] += inv_size;
    }
    ids_.push_back(document_id);
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
                                                     DocumentStatus document_status) const {
    return FindTopDocuments(raw_query,
                            [document_status](int document_id, DocumentStatus status, int rating) {
                                return status == document_status;
                            });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
                                                                                 int document_id) const {
    DocumentIdIsNotNegative(document_id);
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const auto& word : query.plus_words) {
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
    for (const auto& word : query.minus_words) {
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
    return {matched_words, documents_.at(document_id).status};
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