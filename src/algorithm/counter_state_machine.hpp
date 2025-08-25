#ifndef CCWC_COUNTER_STATE_MACHINE_HPP
#define CCWC_COUNTER_STATE_MACHINE_HPP

#include "counter.hpp"

#include <memory>

namespace ccwc::algorithm
{

    /**
     * @brief Abstract base class for FSMs that update character counters.
     *
     * Implements the Chain of Responsibility pattern:
     * each state machine handles its own concern and then passes
     * the byte to the next one in the chain.
     */
    class CounterStateMachine
    {
      private:
        std::unique_ptr<CounterStateMachine> m_next;

      protected:
        void passToNextState(unsigned char byte)
        {
            if (m_next)
            {
                m_next->updateState(byte);
            }
        }

        void passToNextCounter(Counter& counter)
        {
            if (m_next)
            {
                m_next->updateCounter(counter);
            }
        }

        void passToNextReset()
        {
            if (m_next)
            {
                m_next->reset();
            }
        }

      public:
        CounterStateMachine()          = default;
        virtual ~CounterStateMachine() = default;

        CounterStateMachine(const CounterStateMachine&)                    = delete;
        auto operator=(const CounterStateMachine&) -> CounterStateMachine& = delete;
        CounterStateMachine(CounterStateMachine&&)                         = delete;
        auto operator=(CounterStateMachine&&) -> CounterStateMachine&      = delete;

        /**
         * @brief Feed a single byte into the FSM to update its state.
         */
        virtual void updateState(unsigned char byte) = 0;

        /**
         * @brief Update the Counter based on the current FSM state.
         */
        virtual void updateCounter(Counter& counter) = 0;

        /**
         * @brief Reset internal state of FSM.
         */
        virtual void reset() = 0;

        /**
         * @brief Link the next state machine in the chain.
         * @param next The next state machine to link.
         * @return A pointer to the newly linked state machine (for chaining calls).
         */
        auto setNext(std::unique_ptr<CounterStateMachine> next) -> CounterStateMachine*
        {
            m_next = std::move(next);
            return m_next.get();
        }
    };

    /**
     * @brief Build a chain of CounterStateMachine objects.
     *
     * Order of processing:
     *   LinesStateMachine → WordsStateMachine → MultibyteStateMachine → BytesStateMachine
     *
     * @return A unique_ptr to the head of the chain.
     */
    inline auto buildCounterStateMachineChain() -> std::unique_ptr<CounterStateMachine>;

} // namespace ccwc::algorithm

#endif // CCWC_COUNTER_STATE_MACHINE_HPP
