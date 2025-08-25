#include "counter_state_machine.hpp"

#include "counter.hpp"
#include "exception/exception.hpp"

#include <cctype>
#include <clocale>
#include <cwchar>
#include <memory>
#include <vector>

namespace ccwc::algorithm
{

    namespace detail
    {
        /**
         * @brief State machine for counting lines.
         */
        class LineStateMachine : public CounterStateMachine
        {
          private:
            /**
             * @brief The current byte being processed.
             */
            unsigned char m_byte{};

          public:
            /**
             * @brief Update the state of the state machine.
             * @param byte The byte to process.
             */
            void updateState(unsigned char byte) override
            {
                m_byte = byte;
                passToNextState(byte);
            }

            /**
             * @brief Update the counter based on the current state.
             * @param counter The counter to update.
             */
            void updateCounter(Counter& counter) override
            {
                if (m_byte == '\n')
                {
                    counter.lines++;
                }
                passToNextCounter(counter);
            }

            /**
             * @brief Reset the state machine.
             */
            void reset() override
            {
                m_byte = 0;
                passToNextReset();
            }
        };

        /**
         * @brief State machine for counting words.
         */
        class WordStateMachine : public CounterStateMachine
        {
          private:
            /**
             * @brief The current byte being processed.
             */
            unsigned char m_byte{};

            /**
             * @brief Whether the current byte is in a word.
             */
            bool m_inWord{false};

          public:
            /**
             * @brief Update the state of the state machine.
             * @param byte The byte to process.
             */
            void updateState(unsigned char byte) override
            {
                m_byte = byte;
                passToNextState(byte);
            }

            /**
             * @brief Update the counter based on the current state.
             * @param counter The counter to update.
             */
            void updateCounter(Counter& counter) override
            {
                if (std::isspace(m_byte) != 0)
                {
                    m_inWord = false;
                }
                else
                {
                    if (!m_inWord)
                    {
                        counter.words++;
                        m_inWord = true;
                    }
                }
                passToNextCounter(counter);
            }

            /**
             * @brief Reset the state machine.
             */
            void reset() override
            {
                m_byte   = 0;
                m_inWord = false;
                passToNextReset();
            }
        };

        constexpr auto UTF8_LOCALE               = "en_US.UTF-8";
        constexpr auto INVALID_MB_CHAR_RESULT    = static_cast<std::size_t>(-1);
        constexpr auto INCOMPLETE_MB_CHAR_RESULT = static_cast<std::size_t>(-2);

        /**
         * @brief Counts multibyte characters using mbstate_t + mbrtowc_l.
         *
         * Thread-safe because it uses an explicit UTF-8 locale.
         * Works in a chain of responsibility.
         */
        class MultibyteStateMachine : public CounterStateMachine
        {
          private:
            /**
             * @brief The current state of the state machine.
             */
            mbstate_t m_state{{}};

            /**
             * @brief The buffer for the current multibyte character.
             */
            std::vector<char> m_buffer;

            /**
             * @brief The number of bytes in the current multibyte character.
             */
            size_t m_charBytes{0};

            /**
             * @brief Whether the current multibyte character is complete.
             */
            bool m_charComplete{false};

            /**
             * @brief Whether the current multibyte character is invalid.
             */
            bool m_invalid{false};

            /**
             * @brief The locale for the state machine.
             */
            locale_t m_locale;

          public:
            /**
             * @brief Constructor for the MultibyteStateMachine class.
             */
            MultibyteStateMachine() : m_locale(newlocale(LC_CTYPE_MASK, UTF8_LOCALE, nullptr))
            {
                if (m_locale == nullptr)
                {
                    throw ccwc::exception::FileOperationException("Failed to create UTF-8 locale");
                }
                m_buffer.reserve(MB_CUR_MAX); // dynamic buffer
            }

            /**
             * @brief Destructor for the MultibyteStateMachine class.
             */
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

            /**
             * @brief Update the state of the state machine.
             * @param byte The byte to process.
             */
            void updateState(unsigned char byte) override
            {
                m_buffer.push_back(static_cast<char>(byte));

                wchar_t     wideChar{};
                std::size_t result =
                    mbrtowc_l(&wideChar, m_buffer.data(), m_buffer.size(), &m_state, m_locale);

                if (result == INVALID_MB_CHAR_RESULT)
                {
                    // Invalid sequence: reset
                    m_invalid = true;
                    m_buffer.clear();
                    m_state = {};
                }
                else if (result == INCOMPLETE_MB_CHAR_RESULT)
                {
                    // Incomplete sequence: wait for more bytes
                    return;
                }
                else
                {
                    // Completed character
                    m_charComplete = true;
                    m_charBytes    = result;
                    m_buffer.erase(m_buffer.begin(),
                                   m_buffer.begin() +
                                       static_cast<std::vector<char>::difference_type>(result));
                }

                // Pass first byte to next FSM if available
                if (!m_invalid && !m_buffer.empty())
                {
                    passToNextState(static_cast<unsigned char>(m_buffer[0]));
                }
            }

            /**
             * @brief Update the counter based on the current state.
             * @param counter The counter to update.
             */
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

                passToNextCounter(counter);
            }

            /**
             * @brief Reset the state machine.
             */
            void reset() override
            {
                m_buffer.clear();
                m_charComplete = false;
                m_charBytes    = 0;
                m_invalid      = false;
                m_state        = {};

                passToNextReset();
            }
        };

        /**
         * @brief State machine for counting bytes.
         */
        class ByteStateMachine : public CounterStateMachine
        {
          private:
            /**
             * @brief The current byte being processed.
             */
            unsigned char m_byte{};

          public:
            /**
             * @brief Update the state of the state machine.
             * @param byte The byte to process.
             */
            void updateState(unsigned char byte) override
            {
                m_byte = byte;
                passToNextState(byte);
            }

            /**
             * @brief Update the counter based on the current state.
             * @param counter The counter to update.
             */
            void updateCounter(Counter& counter) override
            {
                counter.bytes++;
                passToNextCounter(counter);
            }

            /**
             * @brief Reset the state machine.
             */
            void reset() override
            {
                m_byte = 0;
                passToNextReset();
            }
        };

    } // namespace detail

    inline auto buildCounterStateMachineChain() -> std::unique_ptr<CounterStateMachine>
    {
        auto lines = std::make_unique<detail::LineStateMachine>();

        lines->setNext(std::make_unique<detail::WordStateMachine>())
            ->setNext(std::make_unique<detail::MultibyteStateMachine>())
            ->setNext(std::make_unique<detail::ByteStateMachine>());

        return lines;
    }
} // namespace ccwc::algorithm