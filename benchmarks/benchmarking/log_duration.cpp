#include <benchmarking/log_duration.hpp>

namespace benchmarking {

LogDuration::LogDuration(std::string_view operation_name, std::ostream& output_stream)
        : operation_name_(operation_name)
        , output_stream_(output_stream) {
}

LogDuration::~LogDuration() {
    using namespace std::chrono;
    using namespace std::literals;

    const auto end_time = Clock::now();
    const auto dur = end_time - start_time_;
    output_stream_ << operation_name_
                   << ": "
                   << duration_cast<milliseconds>(dur).count() << " ms"
                   << std::endl;
}

} // namespace benchmarking
