#ifndef SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_
#define SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y

#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)

#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)

#define LOG_DURATION(opName) benchmarking::LogDuration UNIQUE_VAR_NAME_PROFILE(opName)

#define LOG_DURATION_STREAM(opName, stream)                                                        \
    benchmarking::LogDuration UNIQUE_VAR_NAME_PROFILE(opName, stream)

namespace benchmarking {

class LogDuration {
    const std::string_view mOperationName;
    std::ostream& mOutputStream;

    using Clock = std::chrono::steady_clock;
    const Clock::time_point mStartTime = Clock::now();

public: // Constructor/destructor

    explicit LogDuration(std::string_view operationName, std::ostream& outputStream = std::cerr);

    ~LogDuration();
};

} // namespace benchmarking

#endif // SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_
