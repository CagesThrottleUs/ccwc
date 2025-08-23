#include "argument_parser/argument_parser.hpp"

#include <iostream>

auto main(int argc, char** argv) -> int
{
    auto args = ccwc::parseArguments(argc, argv);

    std::cout << "Hello, from ccwc!\n";
    return 0;
}
