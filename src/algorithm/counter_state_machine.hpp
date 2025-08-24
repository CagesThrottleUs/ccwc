#ifndef CCWC_COUNTER_STATE_MACHINE_HPP
#define CCWC_COUNTER_STATE_MACHINE_HPP

#include "counter.hpp"

#include <cctype>
#include <climits>
#include <clocale>
#include <cwchar>
#include <memory>
#include <vector>

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
         * @brief Link the next state machine in the chain.
         * @return pointer to the newly linked state machine (for chaining calls).
         */
        auto setNext(std::unique_ptr<CounterStateMachine> next) -> CounterStateMachine*
        {
            m_next = std::move(next);
            return m_next.get();
        }

      protected:
        std::unique_ptr<CounterStateMachine>
            m_next; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

        void passToNext(unsigned char byte, Counter& counter)
        {
            if (m_next)
            {
                m_next->updateState(byte);
                m_next->updateCounter(counter);
            }
        }
    };

    /**
     * @brief Counts bytes.
     */
    class BytesStateMachine : public CounterStateMachine
    {
      public:
        void updateState(unsigned char byte) override
        {
            m_byte = byte;
        }

        void updateCounter(Counter& counter) override
        {
            counter.bytes++;
            passToNext(m_byte, counter);
        }

      private:
        unsigned char m_byte{};
    };

    /**
     * @brief Counts lines based on newline characters.
     */
    class LinesStateMachine : public CounterStateMachine
    {
      public:
        void updateState(unsigned char byte) override
        {
            m_byte = byte;
        }

        void updateCounter(Counter& counter) override
        {
            if (m_byte == '\n')
            {
                counter.lines++;
            }
            passToNext(m_byte, counter);
        }

      private:
        unsigned char m_byte{};
    };

    /**
     * @brief Counts words by detecting transitions from whitespace → non-whitespace.
     */
    class WordsStateMachine : public CounterStateMachine
    {
      public:
        void updateState(unsigned char byte) override
        {
            m_byte = byte;
        }

        void updateCounter(Counter& counter) override
        {
            if (std::isspace(m_byte) != 0)
            {
                if (m_inWord)
                {
                    m_inWord = false;
                }
            }
            else
            {
                if (!m_inWord)
                {
                    m_inWord = true;
                    counter.words++;
                }
            }
            passToNext(m_byte, counter);
        }

      private:
        unsigned char m_byte{};
        bool          m_inWord{false};
    };

    /**
     * @brief Counts multibyte characters using mbstate_t + mbrtowc_l.
     *
     * Thread-safe because it uses an explicit UTF-8 locale.
     */
    class MultibyteStateMachine : public CounterStateMachine
    {
      public:
        MultibyteStateMachine() : m_locale(newlocale(LC_CTYPE_MASK, "en_US.UTF-8", nullptr))
        {
            if (m_locale == nullptr)
            {
                throw std::runtime_error("Failed to create UTF-8 locale");
            }
            m_buffer.reserve(MB_CUR_MAX); // dynamically allocate buffer
        }

        ~MultibyteStateMachine() override
        {
            if (m_locale != nullptr)
            {
                freelocale(m_locale);
            }
        }

        // Deleted copy/move to avoid accidental sharing of mbstate_t and locale
        MultibyteStateMachine(const MultibyteStateMachine&)                    = delete;
        auto operator=(const MultibyteStateMachine&) -> MultibyteStateMachine& = delete;
        MultibyteStateMachine(MultibyteStateMachine&&)                         = delete;
        auto operator=(MultibyteStateMachine&&) -> MultibyteStateMachine&      = delete;

        void updateState(unsigned char byte) override
        {
            m_buffer.push_back(static_cast<char>(byte));
            wchar_t     wideChar{};
            std::size_t result =
                mbrtowc_l(&wideChar, m_buffer.data(), m_buffer.size(), &m_state, m_locale);

            if (result == INVALID_MB_CHAR_SEQUENCE)
            {
                // Invalid sequence: reset buffer and state
                m_invalid = true;
                m_bufLen  = 0;
                m_buffer.clear();
                m_state = {};
            }
            else if (result == INCOMPLETE_MB_CHAR_SEQUENCE)
            {
                // Incomplete sequence: wait for more bytes
                return;
            }
            else
            {
                // Completed character
                m_charComplete = true;
                m_charBytes    = result;
                m_bufLen       = 0;
                m_buffer.erase(m_buffer.begin(),
                               m_buffer.begin() + static_cast<std::vector<char>::difference_type>(
                                                      result)); // remove consumed bytes
            }
        }

        void updateCounter(Counter& counter) override
        {
            if (m_charComplete)
            {
                if (m_charBytes > 1)
                {
                    counter.multibyte++;
                }
                m_charComplete = false;
            }

            if (!m_invalid && m_next)
            {
                // Pass only the first byte forward
                if (!m_buffer.empty())
                {
                    m_next->updateState(static_cast<unsigned char>(m_buffer[0]));
                }
                m_next->updateCounter(counter);
            }
            m_invalid = false;
        }

      private:
        static constexpr std::size_t INCOMPLETE_MB_CHAR_SEQUENCE = static_cast<std::size_t>(-2);
        static constexpr std::size_t INVALID_MB_CHAR_SEQUENCE    = static_cast<std::size_t>(-1);

        mbstate_t         m_state{{}};
        std::vector<char> m_buffer;
        size_t            m_bufLen{0};
        bool              m_charComplete{false};
        size_t            m_charBytes{0};
        bool              m_invalid{false};
        locale_t          m_locale;
    };

    /**
     * @brief Build a chain of CounterStateMachine objects.
     *
     * Order of processing:
     *   LinesStateMachine → WordsStateMachine → MultibyteStateMachine → BytesStateMachine
     *
     * @return A unique_ptr to the head of the chain.
     */
    inline auto buildCounterStateMachineChain() -> std::unique_ptr<CounterStateMachine>
    {
        // Create the first FSM in the chain
        auto lines = std::make_unique<LinesStateMachine>();

        // Chain the others using setNext
        lines->setNext(std::make_unique<WordsStateMachine>())
            ->setNext(std::make_unique<MultibyteStateMachine>())
            ->setNext(std::make_unique<BytesStateMachine>());

        return lines;
    }

} // namespace ccwc::algorithm

#endif // CCWC_COUNTER_STATE_MACHINE_HPP
