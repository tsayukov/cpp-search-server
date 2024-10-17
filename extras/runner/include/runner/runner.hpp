#ifndef SEARCH_SERVER_RUNNER_RUNNER_HPP_
#define SEARCH_SERVER_RUNNER_RUNNER_HPP_

#include <algorithm>
#include <execution>
#include <iostream>
#include <string_view>
#include <vector>

#define RUN(runnerName)                                                                            \
    namespace runner::namespace_##runnerName {                                                     \
        class FnRunner_##runnerName : public FnRunner {                                            \
        public:                                                                                    \
            std::string_view name() const noexcept override {                                      \
                return #runnerName;                                                                \
            }                                                                                      \
                                                                                                   \
            void run() const override;                                                             \
        };                                                                                         \
                                                                                                   \
        inline const FnRunner_##runnerName singletonFnRunner_##runnerName{};                       \
    } /* namespace runner::namespace_##runnerName */                                               \
                                                                                                   \
    inline void runner::namespace_##runnerName::FnRunner_##runnerName::run() const // {
    // /* code to run */
    // }

namespace runner {

class FnRunner {
public:
    FnRunner();

    virtual ~FnRunner() = default;

    virtual std::string_view name() const noexcept = 0;

    virtual void run() const = 0;

    virtual void prettyRun() const {
        run();
        std::cerr << name() << " OK" << std::endl;
    }
};

class Runner {
    std::vector<FnRunner*> mFnRunners;

public: // Runner

    template <typename ExecutionPolicy = decltype(std::execution::seq)>
    void runAll(const ExecutionPolicy& policy = std::execution::seq) const {
        std::for_each(policy, mFnRunners.begin(), mFnRunners.end(),
                      [](FnRunner* fnRunner) { fnRunner->prettyRun(); });
    }

public: // Modifier

    void add(FnRunner* fnRunner) {
        mFnRunners.push_back(fnRunner);
    }
};

inline Runner singletonRunner{};

inline Runner& getRunner() noexcept {
    return singletonRunner;
}

inline FnRunner::FnRunner() {
    getRunner().add(this);
}

} // namespace runner

#endif // SEARCH_SERVER_RUNNER_RUNNER_HPP_
