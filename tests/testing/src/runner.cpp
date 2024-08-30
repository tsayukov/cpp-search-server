#include <testing/details/runner.hpp>

namespace testing {

Runner singleton_runner;

Runner& GetRunner() noexcept {
    return singleton_runner;
}

} // namespace testing
