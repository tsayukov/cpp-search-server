#include "unit_tests.hpp"
#include "benchmark_tests.hpp"

#include "process_queries.hpp"
#include "search_server/search_server.hpp"

using namespace std;

int main() {
    RunAllTests();
    RunAllBenchmarkTests();
}
