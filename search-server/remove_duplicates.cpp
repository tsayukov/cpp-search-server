#include <algorithm>
#include <iostream>
#include <set>
#include <string>

#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer &server) {
    using namespace std::string_literals;

    std::set<int> ids_to_remove;
    std::set<std::set<std::string>> documents;
    for (const auto id : server) {
        std::set<std::string> words;
        const auto& word_frequencies = server.GetWordFrequencies(id);
        std::transform(word_frequencies.begin(), word_frequencies.end(),
                       std::inserter(words, words.end()),
                       [](const auto& key_value) { return key_value.first; });
        if (documents.count(words) == 0) {
            documents.insert(words);
        } else {
            ids_to_remove.insert(id);
        }
    }

    for (const auto& id : ids_to_remove) {
        server.RemoveDocument(id);
        std::cout << "Found duplicate document id "s << id << "\n"s;
    }
}