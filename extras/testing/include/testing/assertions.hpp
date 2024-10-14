#ifndef SEARCH_SERVER_TESTING_ASSERTIONS_HPP_
#define SEARCH_SERVER_TESTING_ASSERTIONS_HPP_

#include <testing/details/output.hpp>

#define ASSERT_EQUAL(a, b) \
    testing::AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint) \
    testing::AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expr) \
    testing::AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint) \
    testing::AssertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(expression, exception)                                                        \
    do {                                                                                           \
        constexpr auto error_output = []() -> std::ostream& {                                      \
            return std::cerr << __FILE__ << "(" << __LINE__ << "): " << __FUNCTION__ << ": "       \
                             << "ASSERT(" << #expression << ") failed. ";                          \
        };                                                                                         \
        try {                                                                                      \
            expression;                                                                            \
            error_output() << #exception << "must be thrown." << std::endl;                        \
            std::abort();                                                                          \
        } catch (const exception&) {                                                               \
            /* Caught the expected exception */                                                    \
        } catch (...) {                                                                            \
            error_output() << "Incorrect exception. Expect " << #exception << "." << std::endl;    \
            std::abort();                                                                          \
        }                                                                                          \
    } while (false)

namespace testing {

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u,
                     std::string_view t_str_repr, std::string_view u_str_repr,
                     std::string_view file_name, std::string_view fn_name, unsigned line_number,
                     std::string_view hint) {
    using namespace testing::details;
    if (t != u) {
        std::cerr << std::boolalpha
                  << file_name << "(" << line_number << "): " << fn_name << ": "
                  << "ASSERT_EQUAL(" << t_str_repr << ", " << u_str_repr << ") failed: "
                  << t << " != " << u << ".";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

inline void AssertImpl(bool value, std::string_view value_str_repr,
                       std::string_view file_name, std::string_view fn_name, unsigned line_number,
                       std::string_view hint) {
    if (!value) {
        std::cerr << file_name << "(" << line_number << "): " << fn_name << ": "
                  << "ASSERT(" << value_str_repr << ") failed.";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        std::abort();
    }
}

} // namespace testing

#endif // SEARCH_SERVER_TESTING_ASSERTIONS_HPP_
