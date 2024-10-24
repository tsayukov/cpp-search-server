/// \file assertions.hpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_TESTING_ASSERTIONS_HPP_
#define SEARCH_SERVER_TESTING_ASSERTIONS_HPP_

#include <testing/details/output.hpp>

#include <functional>

#define ASSERT_EQUAL(a, b)                                                                         \
    testing::assertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint)                                                              \
    testing::assertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expr)                                                                               \
    testing::assertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint)                                                                    \
    testing::assertImpl(static_cast<bool>(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(expr, exception)                                                              \
    testing::assertThrowImpl<exception>([&] { expr; }, #expr, #exception, __FILE__, __FUNCTION__,  \
                                        __LINE__)

namespace testing {

template <typename T, typename U>
void assertEqualImpl(const T& t,
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

inline void assertImpl(bool value,
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

template <typename Exception>
void assertThrowImpl(std::function<void()> expression,
                     std::string_view expressionStrRepr,
                     std::string_view exceptionStrRepr,
                     std::string_view fileName,
                     std::string_view fnName,
                     unsigned lineNumber) {
    const auto errorOutput = [=]() -> std::ostream& {
        return std::cerr << fileName << "(" << lineNumber << "): " << fnName << ": "
                         << "ASSERT(" << expressionStrRepr << ") failed. ";
    };
    try {
        expression();
        errorOutput() << exceptionStrRepr << "must be thrown." << std::endl;
        std::abort();
    } catch (const Exception&) {
        /* Caught the expected exception */
    } catch (...) {
        errorOutput() << "Incorrect exception. Expect " << exceptionStrRepr << "." << std::endl;
        std::abort();
    }
}

} // namespace testing

#endif // SEARCH_SERVER_TESTING_ASSERTIONS_HPP_
