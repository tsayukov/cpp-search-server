#ifndef SEARCH_SERVER_TESTING_RUNNER_HPP_
#define SEARCH_SERVER_TESTING_RUNNER_HPP_

#include <algorithm>
#include <execution>
#include <iostream>
#include <string_view>
#include <vector>

#define TEST(fn)                                                                                   \
    namespace testing::namespace_##fn {                                                            \
        class Test##fn : public TestCase {                                                         \
        public:                                                                                    \
            Test##fn() {                                                                           \
                GetRunner().AddTest(this);                                                         \
            }                                                                                      \
                                                                                                   \
            std::string_view Name() const noexcept override {                                      \
                return #fn;                                                                        \
            }                                                                                      \
                                                                                                   \
            virtual void Run() const override;                                                     \
        };                                                                                         \
                                                                                                   \
        inline const Test##fn singleton_test_##fn{};                                               \
    }                                                                                              \
                                                                                                   \
    void testing::namespace_##fn::Test##fn::Run() const // expected { /* test code */ }

namespace testing {

class TestCase {
public:
    virtual ~TestCase() = default;

    virtual std::string_view Name() const noexcept = 0;

    virtual void Run() const = 0;
};

class Runner {
    std::vector<TestCase*> test_cases_;

public: // Runner

    template <typename ExecutionPolicy>
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

Runner& GetRunner() noexcept;

} // namespace testing

#endif // SEARCH_SERVER_TESTING_RUNNER_HPP_
