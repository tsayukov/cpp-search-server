#include <search_server/details/string_processing.hpp>

namespace search_server::details {

[[nodiscard]]
std::vector<std::string> splitIntoWords(std::string_view text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(std::move(word));
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(std::move(word));
    }

    return words;
}

[[nodiscard]]
std::vector<std::string_view> splitIntoWordsView(std::string_view text) {
    using pos_t = std::size_t;
    using count_t = std::size_t;

    std::vector<std::string_view> words;

    pos_t pos = text.find_first_not_of(' ');
    text.remove_prefix(pos == std::string_view::npos ? text.size() : count_t(pos));

    while (!text.empty()) {
        pos_t pos_space = text.find(' ');
        words.push_back(text.substr(pos_t(0), count_t(pos_space)));
        pos = text.find_first_not_of(' ', pos_space);
        text.remove_prefix(pos == std::string_view::npos ? text.size() : count_t(pos));
    }

    return words;
}

} // namespace search_server::details
