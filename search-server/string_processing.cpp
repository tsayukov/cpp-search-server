#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
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

std::vector<std::string_view> SplitIntoWordsView(std::string_view text) {
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