/// \file runner.hpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_RUNNER_RUNNER_HPP_
#define SEARCH_SERVER_RUNNER_RUNNER_HPP_

#include <algorithm>
#include <cstring>
#include <execution>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define RUN(runnerName)                                                                            \
    namespace runner::namespace_##runnerName {                                                     \
        class FnRunner_##runnerName : public FnRunner {                                            \
        public:                                                                                    \
            FnRunner_##runnerName() : FnRunner(#runnerName) {}                                     \
                                                                                                   \
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
    FnRunner(std::string_view name);

    virtual ~FnRunner() = default;

    virtual std::string_view name() const noexcept = 0;

    virtual void run() const = 0;

    virtual void prettyRun() const {
        run();
        std::cerr << name() << " OK" << std::endl;
    }
};

class Runner {
    std::unordered_map<std::string_view, FnRunner*> mFnRunners;

public: // Runner

    template <typename ExecutionPolicy>
    void run(const ExecutionPolicy& policy,
             std::unordered_set<std::string> includes,
             std::unordered_set<std::string> excludes) const {
        std::unordered_map<std::string_view, FnRunner*> includedFnRunners;

        for (const auto& name : includes) {
            if (auto it = mFnRunners.find(name); it != mFnRunners.end()) {
                includedFnRunners.emplace(it->first, it->second);
            }
        }

        if (includes.empty()) {
            includedFnRunners = mFnRunners;
        }

        for (const auto& name : excludes) {
            if (auto it = includedFnRunners.find(name); it != includedFnRunners.end()) {
                includedFnRunners.erase(it);
            }
        }

        std::vector<FnRunner*> runners;
        runners.reserve(includedFnRunners.size());
        for (const auto [_, fnRunner] : includedFnRunners) {
            runners.emplace_back(fnRunner);
        }

        std::for_each(policy, runners.begin(), runners.end(),
                      [](FnRunner* fnRunner) { fnRunner->prettyRun(); });
    }

public: // Modifier

    void add(std::string_view name, FnRunner* fnRunner) {
        mFnRunners.emplace(name, fnRunner);
    }
};

inline Runner singletonRunner{};

inline Runner& getRunner() noexcept {
    return singletonRunner;
}

inline FnRunner::FnRunner(std::string_view name) {
    getRunner().add(name, this);
}

class Main {
    bool mIsParallel = false;
    std::unordered_set<std::string> mIncludes, mExcludes;

public:

    void main(int argc, char* argv[]) {
        parse(static_cast<std::size_t>(argc), argv);
        if (mIsParallel) {
            runner::getRunner().run(std::execution::par, mIncludes, mExcludes);
        } else {
            runner::getRunner().run(std::execution::seq, mIncludes, mExcludes);
        }
    }

private:

    void parse(const std::size_t argc, const char* const argv[]) {
        for (std::size_t index = 1; index < argc; ++index) {
            const std::string_view argument = argv[index];
            if (argument.substr(0, 2) == "--") {
                const auto option = argument.substr(2);
                if (option == "parallel") {
                    mIsParallel = true;
                } else if (option == "filter") {
                    const auto count = parseFilter(index + 1, argc, argv);
                    if (count != 0) {
                        index += count;
                    } else {
                        std::cerr << "Missing arguments for `--filter`" << std::endl;
                        return std::abort();
                    }
                } else {
                    std::cerr << "Unknown option: " << argument << std::endl;
                    return std::abort();
                }
            }
        }
    }

    std::size_t parseFilter(std::size_t start, const std::size_t argc, const char* const argv[]) {
        std::size_t count = 0;
        for (std::size_t index = start; index < argc; ++index) {
            const std::string_view argument = argv[index];
            if (argument.substr(0, 2) == "--") {
                break;
            }

            if (argument.substr(0, 1) == "+") {
                if (const std::string name(argument.substr(1)); !name.empty()) {
                    mIncludes.emplace(name);
                }
            } else if (argument.substr(0, 1) == "-") {
                if (const std::string name(argument.substr(1)); !name.empty()) {
                    mExcludes.emplace(name);
                }
            } else {
                // clang-format off
                std::cerr << "Missing '+' or '-' before the runner's name. "
                          << "Got: " << argument << std::endl;
                // clang-format on
                std::abort();
            }

            count += 1;
        }
        return count;
    }
};

} // namespace runner

#endif // SEARCH_SERVER_RUNNER_RUNNER_HPP_
