#include "read_input_functions.h"

#include <iostream>

std::string ReadLine() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    std::cin >> result >> std::ws;
    return result;
}