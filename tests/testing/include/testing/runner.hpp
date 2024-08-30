#ifndef SEARCH_SERVER_TESTING_RUNNER_HPP_
#define SEARCH_SERVER_TESTING_RUNNER_HPP_

#include <algorithm>
#include <execution>
#include <iostream>
#include <string_view>
#include <vector>

#define TEST(test_case_name)                                                                       \
    namespace testing::namespace_##test_case_name {                                                \
        class Test##test_case_name : public TestCase {                                             \
        public:                                                                                    \
            std::string_view Name() const noexcept override {                                      \
                return #test_case_name;                                                            \
            }                                                                                      \
                                                                                                   \
            void Run() const override;                                                             \
        };                                                                                         \
                                                                                                   \
        inline const Test##test_case_name singleton_test_##test_case_name{};                       \
    } /* namespace testing::namespace_##test_case_name */                                          \
                                                                                                   \
    inline void testing::namespace_##test_case_name::Test##test_case_name::Run() const // {
    //     /* test case code */
    // }

namespace testing {

class TestCase {
public:
    TestCase();

    virtual ~TestCase() = default;

    virtual std::string_view Name() const noexcept = 0;

    virtual void Run() const = 0;
};

class Runner {
    std::vector<TestCase*> test_cases_;

public: // Runner

    template <typename ExecutionPolicy = decltype(std::execution::seq)>
    void RunAllTests(const ExecutionPolicy& policy = std::execution::seq) const {
        std::for_each(policy, test_cases_.begin(), test_cases_.end(), [](TestCase* test) {
            test->Run();
            std::cerr << test->Name() << " OK" << std::endl;
        });
    }

public: // Modifier

    void AddTest(TestCase* test) {
        test_cases_.push_back(test);
    }
};

inline Runner singleton_runner;

inline Runner& GetRunner() noexcept {
    return singleton_runner;
}

inline TestCase::TestCase() {
    GetRunner().AddTest(this);
}

} // namespace testing

#endif // SEARCH_SERVER_TESTING_RUNNER_HPP_
