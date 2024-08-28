#include "unit_tests.hpp"
#include "benchmark_tests.hpp"

#include "search_server/search_server.hpp"

using namespace std;

int main() {
    RunAllTests();
    RunAllBenchmarkTests();
}
