/// \file log_duration.hpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_
#define SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y

#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)

#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)

#define LOG_DURATION(outPtrDuration)                                                               \
    benchmarking::OutputPointerLogDuration UNIQUE_VAR_NAME_PROFILE(outPtrDuration)

#define LOG_DURATION_CERR(opName)                                                                  \
    benchmarking::OutputStreamLogDuration UNIQUE_VAR_NAME_PROFILE(opName)

#define LOG_DURATION_STREAM(opName, stream)                                                        \
    benchmarking::OutputStreamLogDuration UNIQUE_VAR_NAME_PROFILE(opName, stream)

namespace benchmarking {

class BaseLogDuration {
    using Clock = std::chrono::steady_clock;
    const Clock::time_point mStartTime = Clock::now();

public: // Duration getter

    template <typename Duration>
    auto getDuration() const {
        const auto endTime = Clock::now();
        return std::chrono::duration_cast<Duration>(endTime - mStartTime);
    }
};

template <typename Duration>
class OutputPointerLogDuration : public BaseLogDuration {
    static_assert(!std::is_const_v<Duration>);

    Duration* const mOutPtrDuration;

public: // Constructor/destructor

    explicit OutputPointerLogDuration(Duration* outPtrDuration) noexcept
            : mOutPtrDuration(outPtrDuration) {}

    ~OutputPointerLogDuration() {
        *mOutPtrDuration = getDuration<Duration>();
    }
};

class OutputStreamLogDuration : public BaseLogDuration {
    const std::string_view mOperationName;
    std::ostream& mOutputStream;

public: // Constructor/destructor

    explicit OutputStreamLogDuration(std::string_view operationName,
                                     std::ostream& outputStream = std::cerr) noexcept;

    ~OutputStreamLogDuration();
};

} // namespace benchmarking

#endif // SEARCH_SERVER_BENCHMARKING_LOG_DURATION_HPP_
