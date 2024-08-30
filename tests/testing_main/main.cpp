#include <testing/runner.hpp>

#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        testing::GetRunner().RunAllTests();
    } else if (std::strcmp(argv[1], "--parallel") == 0) {
        testing::GetRunner().RunAllTests(std::execution::par);
    } else {
        std::cerr << "Unknown option: " << argv[1]  << std::endl;
        std::abort();
    }
}
