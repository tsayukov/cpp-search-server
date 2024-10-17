#include <runner/runner.hpp>

#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        runner::getRunner().runAll();
    } else if (std::strcmp(argv[1], "--parallel") == 0) {
        runner::getRunner().runAll(std::execution::par);
    } else {
        std::cerr << "Unknown option: " << argv[1] << std::endl;
        std::abort();
    }
}
