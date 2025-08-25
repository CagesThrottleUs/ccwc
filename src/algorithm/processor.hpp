#ifndef CCWC_ALGORITHM_PROCESSOR_HPP
#define CCWC_ALGORITHM_PROCESSOR_HPP

#include "argument_parser/argument_parser.hpp"
#include "counter.hpp"
#include "counter_state_machine.hpp"

#include <vector>

namespace ccwc::algorithm
{

    /**
     * @brief Count the number of bytes, words, lines, and multibyte characters in the input data
     * objects.
     * @param inputDataObjects The input data objects to count.
     * @return The counters for each input data object.
     */
    inline auto doCount(const std::vector<ccwc::argument_parser::InputDataObject>& inputDataObjects)
        -> std::vector<Counter>
    {
        std::vector<Counter> counters;
        counters.reserve(inputDataObjects.size());

        auto stateMachine = buildCounterStateMachineChain();

        for (const auto& inputDataObject : inputDataObjects)
        {
            auto counter = Counter();
            while (auto byte = inputDataObject.mInputStream->nextByte())
            {
                if (!byte.has_value())
                {
                    break;
                }
                if (!inputDataObject.mHealthStatus.mIsHealthy)
                {
                    break;
                }
                stateMachine->updateState(byte.value());
                stateMachine->updateCounter(counter);
            }
            stateMachine->reset();
            counters.push_back(counter);
        }

        return counters;
    }
} // namespace ccwc::algorithm

#endif // CCWC_ALGORITHM_PROCESSOR_HPP