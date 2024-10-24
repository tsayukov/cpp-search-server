/// \file testing.hpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_TESTING_TESTING_HPP_
#define SEARCH_SERVER_TESTING_TESTING_HPP_

#include <runner/runner.hpp>
#include <testing/assertions.hpp>

#define TEST(...) RUN(__VA_ARGS__)

#endif // SEARCH_SERVER_TESTING_TESTING_HPP_
