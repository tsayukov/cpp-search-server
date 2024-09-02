#ifndef SEARCH_SERVER_RUNNER_RUNNER_HPP_
#define SEARCH_SERVER_RUNNER_RUNNER_HPP_

#include <algorithm>
#include <execution>
#include <iostream>
#include <string_view>
#include <vector>

#define RUN(name)                                                                                  \
    namespace runner::namespace_##name {                                                           \
        class FnRunner##name : public FnRunner {                                                   \
        public:                                                                                    \
            std::string_view Name() const noexcept override {                                      \
                return #name;                                                                      \
            }                                                                                      \
                                                                                                   \
            void Run() const override;                                                             \
        };                                                                                         \
                                                                                                   \
        inline const FnRunner##name singleton_fn_runner_##name{};                                  \
    } /* namespace runner::namespace_##name */                                                     \
                                                                                                   \
    inline void runner::namespace_##name::FnRunner##name::Run() const // {
    //     /* code to run */
    // }

namespace runner {

class FnRunner {
public:
    FnRunner();

    virtual ~FnRunner() = default;

    virtual std::string_view Name() const noexcept = 0;

    virtual void Run() const = 0;

    virtual void PrettyRun() const {
        Run();
        std::cerr << Name() << " OK" << std::endl;
    }
};

class Runner {
    std::vector<FnRunner*> fn_runners_;

public: // Runner

    template <typename ExecutionPolicy = decltype(std::execution::seq)>
    void RunAll(const ExecutionPolicy& policy = std::execution::seq) const {
        std::for_each(policy, fn_runners_.begin(), fn_runners_.end(), [](FnRunner* fn_runner) {
            fn_runner->PrettyRun();
        });
    }

public: // Modifier

    void Add(FnRunner* fn_runner) {
        fn_runners_.push_back(fn_runner);
    }
};

inline Runner singleton_runner{};

inline Runner& GetRunner() noexcept {
    return singleton_runner;
}

inline FnRunner::FnRunner() {
    GetRunner().Add(this);
}

} // namespace runner

#endif // SEARCH_SERVER_RUNNER_RUNNER_HPP_
