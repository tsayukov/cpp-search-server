#include "unit_tests.h"
#include "benchmark_tests.h"

#include "remove_duplicates.h"
#include "process_queries.h"
#include "search_server.h"

using namespace std;

int main() {
    RunAllTests();
    RunAllBenchmarkTests();
}