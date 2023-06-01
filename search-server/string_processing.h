#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> SplitIntoWords(std::string_view text);

std::vector<std::string_view> SplitIntoWordsView(std::string_view text);

template<typename ExecutionPolicy, typename ContiguousContainer>
void RemoveDuplicateWords(const ExecutionPolicy& policy, ContiguousContainer& container) {
    static_assert(std::is_convertible_v<typename ContiguousContainer::value_type, std::string_view>);

    std::sort(policy, container.begin(), container.end());
    auto begin_of_words_to_remove = std::unique(container.begin(), container.end());
    container.erase(begin_of_words_to_remove, container.end());
}