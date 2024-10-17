#ifndef SEARCH_SERVER_TESTING_ASSERTIONS_HPP_
#define SEARCH_SERVER_TESTING_ASSERTIONS_HPP_

#include <testing/details/output.hpp>

#define ASSERT_EQUAL(a, b)                                                                         \
    testing::AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint)                                                              \
    testing::AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expr)                                                                               \
    testing::AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint)                                                                    \
    testing::AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(expression, exception)                                                        \
    do {                                                                                           \
        constexpr auto errorOutput = []() -> std::ostream& {                                       \
            return std::cerr << __FILE__ << "(" << __LINE__ << "): " << __FUNCTION__ << ": "       \
                             << "ASSERT(" << #expression << ") failed. ";                          \
        };                                                                                         \
        try {                                                                                      \
            expression;                                                                            \
            errorOutput() << #exception << "must be thrown." << std::endl;                         \
            std::abort();                                                                          \
        } catch (const exception&) {                                                               \
            /* Caught the expected exception */                                                    \
        } catch (...) {                                                                            \
            errorOutput() << "Incorrect exception. Expect " << #exception << "." << std::endl;     \
            std::abort();                                                                          \
        }                                                                                          \
    } while (false)

namespace testing {

template <typename T, typename U>
void AssertEqualImpl(const T& t,
                     const U& u,
                     std::string_view tStrRepr,
                     std::string_view uStrRepr,
                     std::string_view fileName,
                     std::string_view fnName,
                     unsigned lineNumber,
                     std::string_view hint) {
    using namespace testing::details;
    if (t != u) {
        // clang-format off
        std::cerr << std::boolalpha << fileName << "(" << lineNumber << "): " << fnName << ": "
                  << "ASSERT_EQUAL(" << tStrRepr << ", " << uStrRepr << ") failed: "
                  << t << " != " << u << ".";
        // clang-format on
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

inline void AssertImpl(bool value,
                       std::string_view valueStrRepr,
                       std::string_view fileName,
                       std::string_view fnName,
                       unsigned lineNumber,
                       std::string_view hint) {
    if (!value) {
        std::cerr << fileName << "(" << lineNumber << "): " << fnName << ": "
                  << "ASSERT(" << valueStrRepr << ") failed.";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

} // namespace testing

#endif // SEARCH_SERVER_TESTING_ASSERTIONS_HPP_
