#include "counter_state_machine.hpp"

#include "counter.hpp"
#include "exception/exception.hpp"

#include <boost/locale.hpp>
#include <cctype>
#include <cstdlib>
#include <locale>
#include <memory>
#include <string>
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

            void finalize(Counter& counter) override
            {
                passToNextFinalize(counter);
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

            void finalize(Counter& counter) override
            {
                passToNextFinalize(counter);
            }
        };

        /**
         * @brief CounterStateMachine implementation for counting Unicode code points using
         * boost.locale.
         *
         * Reads bytes one by one, buffers them into a UTF-8 string efficiently,
         * then converts the buffered string to a wide string using boost.locale in finalize(),
         * adding the code point count to the counter.
         */
        class MultibyteStateMachine : public CounterStateMachine
        {
          private:
            std::string m_buffer; // Buffer UTF-8 bytes from input
            std::locale m_locale; // Locale for conversion

            static constexpr std::size_t MAX_BUFFER_SIZE = 1048576;

            void flushBuffer(Counter& counter)
            {
                if (m_buffer.empty())
                {
                    return;
                }

                try
                {
                    std::string encoding = std::use_facet<boost::locale::info>(m_locale).encoding();
                    std::wstring wideStr = boost::locale::conv::to_utf<wchar_t>(m_buffer, encoding);
                    counter.multibyte += wideStr.size();
                    m_buffer.clear();
                }
                catch (const boost::locale::conv::conversion_error&)
                {
                    counter.multibyte += 1;
                    m_buffer.clear();
                }
            }

          public:
            // Construct with locale built from system/user environment
            MultibyteStateMachine() : m_locale(boost::locale::generator().generate(""))
            {
            }

            // Feed one byte into the buffer
            void updateState(unsigned char byte) override
            {
                m_buffer.push_back(static_cast<char>(byte));
                passToNextState(byte);
            }

            // No per-byte counting here, counting happens on finalize()
            void updateCounter(Counter& counter) override
            {
                if (m_buffer.size() >= MAX_BUFFER_SIZE)
                {
                    flushBuffer(counter);
                }
                passToNextCounter(counter);
            }

            void reset() override
            {
                m_buffer.clear();
                passToNextReset();
            }

            void finalize(Counter& counter) override
            {
                flushBuffer(counter);
                m_buffer.clear();
                passToNextFinalize(counter);
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

            void finalize(Counter& counter) override
            {
                passToNextFinalize(counter);
            }
        };

    } // namespace detail

    /**
     * @brief Build a chain of CounterStateMachine objects.
     *
     * Order of processing:
     *   LinesStateMachine → WordsStateMachine → MultibyteStateMachine → BytesStateMachine
     *
     * @return A unique_ptr to the head of the chain.
     */
    auto buildCounterStateMachineChain() -> std::unique_ptr<CounterStateMachine>
    {
        auto lines = std::make_unique<detail::LineStateMachine>();

        lines->setNext(std::make_unique<detail::WordStateMachine>())
            ->setNext(std::make_unique<detail::MultibyteStateMachine>())
            ->setNext(std::make_unique<detail::ByteStateMachine>());

        return lines;
    }
} // namespace ccwc::algorithm