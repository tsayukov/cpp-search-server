#include <runner/runner.hpp>

#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        runner::GetRunner().RunAll();
    } else if (std::strcmp(argv[1], "--parallel") == 0) {
        runner::GetRunner().RunAll(std::execution::par);
    } else {
        std::cerr << "Unknown option: " << argv[1]  << std::endl;
        std::abort();
    }
}
