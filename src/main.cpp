#include "algorithm/processor.hpp"
#include "argument_parser/argument_parser.hpp"

#include <iostream>
#include <stdexcept>

auto main(int argc, char** argv) noexcept -> int
{

    try
    {
        auto args = ccwc::parseArguments(argc, argv);

        auto counters = ccwc::algorithm::doCount(args.inputDataObjects());

        args.formatOutput(counters);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
    }

    // std::cout << "Hello, from ccwc!\n";
    return 0;
}
