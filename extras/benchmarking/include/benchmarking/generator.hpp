/// \file generator.hpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_BENCHMARKING_GENERATOR_HPP_
#define SEARCH_SERVER_BENCHMARKING_GENERATOR_HPP_

#include <random>

namespace benchmarking {

class BaseGenerator {
protected:
    inline static std::random_device mRandomDevice;
    inline static auto mGenerator = std::mt19937(mRandomDevice());
};

template <typename T>
class Generator : public BaseGenerator {
public:
    static T get(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
        return std::uniform_int_distribution<T>(min, max)(mGenerator);
    }
};

template <>
class Generator<char> : public BaseGenerator {
public:
    static char get(char min = 'a', char max = 'z') {
        return static_cast<char>(std::uniform_int_distribution<int>(min, max)(mGenerator));
    }
};

template <>
class Generator<double> : public BaseGenerator {
public:
    static double get(double min = std::numeric_limits<double>::min(),
                      double max = std::numeric_limits<double>::max()) {
        return std::uniform_real_distribution<double>(min, max)(mGenerator);
    }
};

} // namespace benchmarking

#endif // SEARCH_SERVER_BENCHMARKING_GENERATOR_HPP_
