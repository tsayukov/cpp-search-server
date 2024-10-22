#include <benchmarking/log_duration.hpp>

namespace benchmarking {

OutputStreamLogDuration::OutputStreamLogDuration(std::string_view operationName,
                                                 std::ostream& outputStream) noexcept
        : mOperationName(operationName)
        , mOutputStream(outputStream) {}

OutputStreamLogDuration::~OutputStreamLogDuration() {
    const auto duration = getDuration<std::chrono::milliseconds>().count();
    mOutputStream << mOperationName << ": " << duration << " ms" << std::endl;
}

} // namespace benchmarking
