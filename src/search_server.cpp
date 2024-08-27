#include <search_server/search_server.hpp>

#include <numeric>
#include <stdexcept>
#include <execution>

using namespace std::string_literals;

// Constructors

SearchServer::SearchServer(std::string_view stop_words)
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

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
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

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    auto document_iter = documents_.find(document_id);
    if (document_iter == documents_.end()) {
        return;
    }

    const auto& document_data = document_iter->second;

    std::vector<std::string_view> word_views(document_data.word_frequencies.size());
    std::transform(
            document_data.word_frequencies.cbegin(), document_data.word_frequencies.cend(),
            word_views.begin(),
            [](const auto& key_value) {
                return key_value.first;
            });

    std::for_each(
            std::execution::par,
            word_views.cbegin(), word_views.cend(),
            [this, document_id](const auto word_view) {
                auto& documents_with_that_word = word_to_document_frequencies_.find(word_view)->second;
                documents_with_that_word.erase(document_id);
            });

    documents_.erase(document_iter);
    document_ids_.erase(document_id);
}

// Search

[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                                   DocumentStatus document_status) const {
    return FindTopDocuments(
            raw_query,
            [document_status](int /*document_id*/, DocumentStatus status, int /*rating*/) {
                return status == document_status;
            });
}

[[nodiscard]] std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::kActual);
}

[[nodiscard]] SearchServer::MatchingWordsAndDocStatus SearchServer::MatchDocument(
        std::string_view raw_query, int document_id) const {
    CheckDocumentIdIsNotNegative(document_id);
    CheckDocumentIdExists(document_id);

    const auto query = ParseQuery(std::execution::seq, raw_query, WordsRepeatable::kNo);
    const auto& document_data = documents_.at(document_id);
    const auto& word_frequencies_in_that_documents = document_data.word_frequencies;

    std::vector<std::string_view> matched_words;
    for (const auto minus_word_view : query.minus_words) {
        auto iter = word_frequencies_in_that_documents.find(minus_word_view);
        if (iter != word_frequencies_in_that_documents.end()) {
            return make_tuple(std::move(matched_words), document_data.status);
        }
    }
    for (const auto plus_word_view : query.plus_words) {
        auto iter = word_frequencies_in_that_documents.find(plus_word_view);
        if (iter != word_frequencies_in_that_documents.end()) {
            matched_words.emplace_back(plus_word_view);
        }
    }

    return make_tuple(std::move(matched_words), document_data.status);
}

[[nodiscard]] SearchServer::MatchingWordsAndDocStatus SearchServer::MatchDocument(
        const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const {
    return MatchDocument(raw_query, document_id);
}

[[nodiscard]] SearchServer::MatchingWordsAndDocStatus SearchServer::MatchDocument(
        const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const {
    CheckDocumentIdIsNotNegative(document_id);
    CheckDocumentIdExists(document_id);

    const auto query = ParseQuery(std::execution::seq, raw_query, WordsRepeatable::kYes);
    const auto& document_data = documents_.at(document_id);
    const auto& word_frequencies_in_that_documents = document_data.word_frequencies;

    std::vector<std::string_view> matched_words;
    auto is_that_document_has_word = [&word_frequencies_in_that_documents](const auto word_view) {
        return word_frequencies_in_that_documents.count(word_view) > 0;
    };

    const bool that_document_has_minus_word = std::any_of(
            std::execution::par,
            query.minus_words.begin(), query.minus_words.end(),
            is_that_document_has_word);
    if (that_document_has_minus_word) {
        return make_tuple(std::move(matched_words), document_data.status);
    }

    matched_words.resize(query.plus_words.size());
    auto begin_of_matched_words_to_remove = std::copy_if(
            query.plus_words.begin(), query.plus_words.end(),
            matched_words.begin(),
            is_that_document_has_word);

    matched_words.erase(begin_of_matched_words_to_remove, matched_words.end());
    RemoveDuplicateWords(std::execution::par, matched_words);

    return make_tuple(std::move(matched_words), document_data.status);
}

[[nodiscard]] std::vector<Document> SearchServer::PrepareResult(
        const std::map<int, double>& document_to_relevance) const {
    std::vector<Document> result;
    result.reserve(document_to_relevance.size());
    for (const auto [document_id, relevance] : document_to_relevance) {
        result.emplace_back(document_id, relevance, documents_.at(document_id).rating);
    }
    return result;
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

void SearchServer::CheckDocumentIdExists(int document_id) const {
    if (documents_.count(document_id) == 0) {
        throw std::invalid_argument("The passed document id doesn't exist"s);
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

[[nodiscard]] double SearchServer::ComputeInverseDocumentFrequency(std::size_t docs_with_the_word) const {
    return std::log(GetDocumentCount() / static_cast<double>(docs_with_the_word));
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
