#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>

#define ASSERT_EQUAL(a, b) \
unit_test_tools::AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) \
unit_test_tools::AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expr) \
unit_test_tools::AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) \
unit_test_tools::AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(func, exception)                                                    \
    try {                                                                                \
        func;                                                                            \
        std::cerr << __FILE__ << "("s << __LINE__ << "): "s << __FUNCTION__ << ": "s;    \
        std::cerr << "ASSERT("s << #func << ") failed. "s;                               \
        std::cerr << #exception << "must be thrown."s << std::endl;                      \
        abort();                                                                         \
    } catch (const exception&) {                                                         \
    } catch (...) {                                                                      \
        std::cerr << __FILE__ << "("s << __LINE__ << "): "s << __FUNCTION__ << ": "s;    \
        std::cerr << "ASSERT("s << #func << ") failed. "s;                               \
        std::cerr << "Incorrect exception. Expect "s << #exception << "."s << std::endl; \
        std::abort();                                                                    \
    }

#define RUN_TEST(func) \
unit_test_tools::RunTestImpl((func), #func)

namespace unit_test_tools {

using namespace std::string_literals;

template <typename Container>
std::ostream& PrintContainer(std::ostream& output, const Container& container, const std::string& sep = ", "s);

template <typename T>
std::ostream& operator<<(std::ostream& output, const std::vector<T>& vector) {
    output << "["s;
    PrintContainer(output, vector);
    return output << "]"s;
}

template <typename T>
std::ostream& operator<<(std::ostream& output, const std::set<T>& set) {
    output << "{"s;
    PrintContainer(output, set);
    return output << "}"s;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& output, const std::map<K, V>& map) {
    output << "{"s;
    PrintContainer(output, map);
    return output << "}"s;
}

template <typename F, typename S>
std::ostream& operator<<(std::ostream& output, const std::pair<F, S>& pair) {
    return output << pair.first << ": "s << pair.second;
}

template <typename Container>
std::ostream& PrintContainer(std::ostream& output, const Container& container, const std::string& sep) {
    auto iter = std::begin(container);
    const auto end_iter = std::end(container);
    if (iter == end_iter) {
        return output;
    }
    output << *iter;
    ++iter;
    for (; iter != end_iter; ++iter) {
        output << sep << *iter;
    }
    return output;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str_repr, const std::string& u_str_repr,
                     const std::string& file_name, const std::string& func_name, unsigned line_number,
                     const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file_name << "("s << line_number << "): "s << func_name << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str_repr << ", "s << u_str_repr << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

inline void AssertImpl(bool value, const std::string& value_str_repr,
                       const std::string& file_name, const std::string& func_name, unsigned line_number,
                       const std::string& hint) {
    if (!value) {
        std::cerr << file_name << "("s << line_number << "): "s << func_name << ": "s;
        std::cerr << "ASSERT("s << value_str_repr << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

template <typename Func>
void RunTestImpl(Func func, const std::string& func_name) {
    func();
    std::cerr << func_name << " OK"s << std::endl;
}

} // namespace unit_test_tools