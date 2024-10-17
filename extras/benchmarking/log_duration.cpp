#include <benchmarking/log_duration.hpp>

namespace benchmarking {

LogDuration::LogDuration(std::string_view operationName, std::ostream& outputStream)
        : mOperationName(operationName)
        , mOutputStream(outputStream) {}

LogDuration::~LogDuration() {
    using namespace std::chrono;
    using namespace std::literals;

    const auto endTime = Clock::now();
    const auto dur = endTime - mStartTime;
    // clang-format off
    mOutputStream << mOperationName
                  << ": "
                  << duration_cast<milliseconds>(dur).count() << " ms"
                  << std::endl;
    // clang-format on
}

} // namespace benchmarking
