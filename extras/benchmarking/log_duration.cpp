/// \file log_duration.cpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

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
