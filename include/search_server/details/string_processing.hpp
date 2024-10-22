#ifndef SEARCH_SERVER_DETAILS_STRING_PROCESSING_HPP_
#define SEARCH_SERVER_DETAILS_STRING_PROCESSING_HPP_

#include <search_server/export.hpp>

#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace search_server::details {

template <typename Container, typename = void>
inline constexpr bool kContainsStringViewLike = false;

template <typename Container>
inline constexpr bool kContainsStringViewLike
// clang-format off
        < Container
        , std::enable_if_t
                < std::is_convertible_v
                        < typename std::decay_t<Container>::value_type
                        , std::string_view
                        >
                >
        > = true;
// clang-format on

[[nodiscard]]
SEARCH_SERVER_EXPORT std::vector<std::string> splitIntoWords(std::string_view text);

[[nodiscard]]
SEARCH_SERVER_EXPORT std::vector<std::string_view> splitIntoWordsView(std::string_view text);

template <typename ExecutionPolicy, typename ContiguousContainer>
void removeDuplicateWords(const ExecutionPolicy& policy, ContiguousContainer& container) {
    static_assert(kContainsStringViewLike<ContiguousContainer>);

    std::sort(policy, container.begin(), container.end());
    auto beginOfWordsToRemove = std::unique(container.begin(), container.end());
    container.erase(beginOfWordsToRemove, container.end());
}

} // namespace search_server::details

#endif // SEARCH_SERVER_DETAILS_STRING_PROCESSING_HPP_
