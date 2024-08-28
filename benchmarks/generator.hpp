#pragma once

#include <random>

class BaseGenerator {
protected:
    inline static std::random_device random_device_;
    inline static auto generator_ = std::mt19937(random_device_());
};

template<typename T>
class Generator : public BaseGenerator {
public:
    static T Get(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
        return std::uniform_int_distribution<T>(min, max)(generator_);
    }
};

template<>
class Generator<char> : public BaseGenerator {
public:
    static char Get(char min = 'a', char max = 'z') {
        return static_cast<char>(std::uniform_int_distribution<int>(min, max)(generator_));
    }
};

template<>
class Generator<double> : public BaseGenerator {
public:
    static double Get(double min = std::numeric_limits<double>::min(),
                      double max = std::numeric_limits<double>::max()) {
        return std::uniform_real_distribution<double>(min, max)(generator_);
    }
};
