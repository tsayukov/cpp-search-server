#ifndef SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_
#define SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y

#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)

#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)

#define LOG_DURATION(op_name) \
    benchmarking::LogDuration UNIQUE_VAR_NAME_PROFILE(op_name)

#define LOG_DURATION_STREAM(op_name, stream) \
    benchmarking::LogDuration UNIQUE_VAR_NAME_PROFILE(op_name, stream)

namespace benchmarking {

class LogDuration {
    const std::string_view operation_name_;
    std::ostream& output_stream_;

    using Clock = std::chrono::steady_clock;
    const Clock::time_point start_time_ = Clock::now();

public: // Constructor/destructor

    explicit LogDuration(std::string_view operation_name, std::ostream& output_stream = std::cerr);

    ~LogDuration();
};

} // namespace benchmarking

#endif // SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_
