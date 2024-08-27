#ifndef SEARCH_SERVER_DETAILS_STRING_PROCESSING_HPP_
#define SEARCH_SERVER_DETAILS_STRING_PROCESSING_HPP_

#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace search_server::details {

template <typename Container>
inline constexpr bool kContainsStringViewLike = std::is_convertible_v
        < typename std::decay_t<Container>::value_type
        , std::string_view
        >;

[[nodiscard]]
std::vector<std::string> SplitIntoWords(std::string_view text);

[[nodiscard]]
std::vector<std::string_view> SplitIntoWordsView(std::string_view text);

template <typename ExecutionPolicy, typename ContiguousContainer>
void RemoveDuplicateWords(const ExecutionPolicy& policy, ContiguousContainer& container) {
    static_assert(kContainsStringViewLike<ContiguousContainer>);

    std::sort(policy, container.begin(), container.end());
    auto begin_of_words_to_remove = std::unique(container.begin(), container.end());
    container.erase(begin_of_words_to_remove, container.end());
}

} // namespace search_server::details

#endif // SEARCH_SERVER_DETAILS_STRING_PROCESSING_HPP_
