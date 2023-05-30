#include "search_server.h"

#include <numeric>
#include <stdexcept>
#include <execution>

using namespace std::string_literals;

// Constructor

SearchServer::SearchServer(const std::string& stop_words)
        : SearchServer(SplitIntoWords(stop_words)) {
}

// Capacity and Lookup

[[nodiscard]] int SearchServer::GetDocumentCount() const noexcept {
    return static_cast<int>(documents_.size());
}

[[nodiscard]] const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string_view, double> empty_map;

    if (auto it = documents_.find(document_id); it != documents_.end()) {
        return it->second.word_frequencies;
    }
    return empty_map;
}

// Iterators

[[nodiscard]] std::set<int>::const_iterator SearchServer::begin() const noexcept {
    return document_ids_.begin();
}

[[nodiscard]] std::set<int>::const_iterator SearchServer::end() const noexcept {
    return document_ids_.end();
}

// Modification

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                               const std::vector<int>& ratings) {
    CheckDocumentIdIsNotNegative(document_id);
    CheckDocumentIdDoesntExist(document_id);

    auto words = SplitIntoWordsNoStop(document);

    document_ids_.insert(document_id);
    auto& document_data = documents_[document_id];
    document_data.rating = ComputeAverageRating(ratings);
    document_data.status = status;

    const double inv_size = 1.0 / static_cast<double>(words.size());
    for (auto& word : words) {
        auto iter = word_to_document_frequencies_.find(word);
        if (iter == word_to_document_frequencies_.end()) {
            auto doc_frequency = std::map<int, double>{{document_id, inv_size}};
            auto [insert_pos, _] = word_to_document_frequencies_.emplace(std::move(word), std::move(doc_frequency));
            iter = insert_pos;
        } else {
            auto& doc_frequency = iter->second;
            doc_frequency[document_id] += inv_size;
        }
        const std::string_view word_view = iter->first;
        document_data.word_frequencies[word_view] += inv_size;
    }
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

// Search

[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
                                                                   DocumentStatus document_status) const {
    return FindTopDocuments(raw_query,
                            [document_status](int /*document_id*/, DocumentStatus status, int /*rating*/) {
                                return status == document_status;
                            });
}

[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

[[nodiscard]] std::tuple<std::vector<std::string>, DocumentStatus>
SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    CheckDocumentIdIsNotNegative(document_id);

    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const auto& word : query.plus_words) {
        auto iter = word_to_document_frequencies_.find(word);
        if (iter == word_to_document_frequencies_.end()) {
            continue;
        }
        if (const auto& document_frequencies = iter->second; document_frequencies.count(document_id) > 0) {
            matched_words.emplace_back(word);
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
    return make_tuple(std::move(matched_words), documents_.at(document_id).status);
}

// Checks

void SearchServer::StringHasNotAnyForbiddenChars(std::string_view s) {
    if (std::any_of(s.begin(), s.end(),
                    [](const char c) {
                        return iscntrl(static_cast<unsigned char>(c));
                    })) {
        throw std::invalid_argument("Stop words contain forbidden characters from 0x00 to 0x1F"s);
    }
}

void SearchServer::CheckDocumentIdIsNotNegative(int document_id) {
    if (document_id < 0) {
        throw std::invalid_argument("The negative document id"s);
    }
}

void SearchServer::CheckDocumentIdDoesntExist(int document_id) const {
    if (documents_.count(document_id) > 0) {
        throw std::invalid_argument("The passed document id already exists"s);
    }
}

[[nodiscard]] bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}

// Metric computation

[[nodiscard]] int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    const int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

[[nodiscard]] double SearchServer::ComputeInverseDocumentFrequency(std::size_t number_of_docs_with_the_word) const {
    return std::log(GetDocumentCount() / static_cast<double>(number_of_docs_with_the_word));
}

// Parsing

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string> result;
    auto words = SplitIntoWords(text);
    for (auto& word : words) {
        StringHasNotAnyForbiddenChars(word);
        if (!IsStopWord(word)) {
            result.push_back(std::move(word));
        }
    }
    return result;
}

[[nodiscard]] SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view word) const {
    bool is_minus = (word[0] == '-');
    if (is_minus) {
        if (word.size() == 1 || word[1] == '-') {
            throw std::invalid_argument("Incorrect form of minus words"s);
        }
        word = word.substr(1);
    }
    return QueryWord{word, is_minus, IsStopWord(word)};
}

[[nodiscard]] SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
    Query query;
    const auto words = SplitIntoWordsView(text);
    for (const auto word : words) {
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