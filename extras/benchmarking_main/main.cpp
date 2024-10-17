#include <runner/runner.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        runner::getRunner().runAll();
    } else {
        std::cerr << "Unknown option: " << argv[1] << std::endl;
        std::abort();
    }
}
