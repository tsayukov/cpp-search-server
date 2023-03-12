#include <iostream>
#include <map>
#include <set>
#include <string>

#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer &server) {
    using namespace std::string_literals;

    std::map<std::set<std::string>, std::set<int>> duplicate_counter;
    for (const auto id : server) {
        std::set<std::string> words;
        const auto& word_frequencies = server.GetWordFrequencies(id);
        for (const auto& [word, _] : word_frequencies) {
            words.insert(word);
        }
        duplicate_counter[words].insert(id);
    }

    for (auto& [_, ids] : duplicate_counter) {
        while (ids.size() > 1) {
            auto pos = --(ids.end());
            const auto id = *pos;
            ids.erase(pos);
            server.RemoveDocument(id);
            std::cout << "Found duplicate document id "s << id << "\n"s;
        }
    }
}