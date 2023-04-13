#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y

#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)

#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)

#define LOG_DURATION(op_name) LogDuration UNIQUE_VAR_NAME_PROFILE(op_name)

#define LOG_DURATION_STREAM(op_name, stream) LogDuration UNIQUE_VAR_NAME_PROFILE(op_name, stream)

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;

    explicit LogDuration(std::string_view operation_name, std::ostream& output_stream = std::cerr);

    ~LogDuration();

private:
    const std::string_view operation_name_;
    std::ostream& output_stream_;
    const Clock::time_point start_time_ = Clock::now();
};

LogDuration::LogDuration(std::string_view operation_name, std::ostream& output_stream)
        : operation_name_(operation_name)
        , output_stream_(output_stream) {
}

LogDuration::~LogDuration() {
    using namespace std::chrono;
    using namespace std::literals;

    const auto end_time = Clock::now();
    const auto dur = end_time - start_time_;
    output_stream_ << operation_name_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
}