#ifndef SEARCH_SERVER_TESTING_DETAILS_OUTPUT_HPP_
#define SEARCH_SERVER_TESTING_DETAILS_OUTPUT_HPP_

#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace testing::details {

template <typename C>
std::ostream& printContainer(std::ostream& output, const C& container, std::string_view sep = ", ");

template <typename T>
std::ostream& operator<<(std::ostream& output, const std::vector<T>& vector) {
    output << "[";
    printContainer(output, vector);
    return output << "]";
}

template <typename T>
std::ostream& operator<<(std::ostream& output, const std::set<T>& set) {
    output << "{";
    printContainer(output, set);
    return output << "}";
}

template <typename T>
std::ostream& operator<<(std::ostream& output, const std::unordered_set<T>& set) {
    output << "{";
    printContainer(output, set);
    return output << "}";
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& output, const std::map<K, V>& map) {
    output << "{";
    printContainer(output, map);
    return output << "}";
}

template <typename F, typename S>
std::ostream& operator<<(std::ostream& output, const std::pair<F, S>& pair) {
    return output << pair.first << ": " << pair.second;
}

template <typename C>
std::ostream& printContainer(std::ostream& output, const C& container, std::string_view sep) {
    using std::begin, std::end;
    auto iter = begin(container);
    const auto endIter = end(container);
    if (iter == endIter) {
        return output;
    }
    output << *iter;
    ++iter;
    for (; iter != endIter; ++iter) {
        output << sep << *iter;
    }
    return output;
}

} // namespace testing::details

#endif // SEARCH_SERVER_TESTING_DETAILS_OUTPUT_HPP_
