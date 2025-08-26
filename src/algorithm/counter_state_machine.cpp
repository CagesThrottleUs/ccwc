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

            static constexpr std::size_t   MAX_BUFFER_SIZE = 4096;
            static constexpr unsigned char UTF8_MASK_1     = 0x80; // 1000 0000
            static constexpr unsigned char UTF8_MASK_3     = 0xE0; // 1110 0000
            static constexpr unsigned char UTF8_MASK_4     = 0xF0; // 1111 0000
            static constexpr unsigned char UTF8_MASK_5     = 0xF8; // 1111 1000
            static constexpr unsigned char UTF8_CONT_MASK  = 0xC0; // 1100 0000
            static constexpr unsigned char UTF8_CONT_VALUE = 0x80; // 1000 0000

            // Helper to determine UTF-8 character length from first byte
            [[nodiscard]] static auto utf8CharLength(unsigned char firstByte) -> std::size_t
            {
                if ((firstByte & UTF8_MASK_1) == 0x00)
                {
                    return 1; // ASCII
                }

                if ((firstByte & UTF8_MASK_3) == UTF8_CONT_MASK)
                {
                    return 2; // 2-byte sequence
                }

                if ((firstByte & UTF8_MASK_4) == UTF8_MASK_3)
                {
                    return 3; // 3-byte sequence
                }

                if ((firstByte & UTF8_MASK_5) == UTF8_MASK_4)
                {
                    return 4; // 4-byte sequence
                }

                return 0; // Invalid leading byte
            }

            // Returns number of bytes corresponding to all full UTF-8 characters at the start of
            // buffer
            [[nodiscard]] auto fullCharsByteCount() const -> std::size_t
            {
                std::size_t pos = 0;
                std::size_t len = m_buffer.size();

                while (pos < len)
                {
                    auto        firstByte = static_cast<unsigned char>(m_buffer[pos]);
                    std::size_t charLen   = utf8CharLength(firstByte);

                    if (charLen == 0 || pos + charLen > len)
                    {
                        // Either invalid start byte or incomplete char
                        break;
                    }
                    // Check continuation bytes validity
                    for (std::size_t i = 1; i < charLen; ++i)
                    {
                        auto ele = static_cast<unsigned char>(m_buffer[pos + i]);
                        if ((ele & UTF8_CONT_MASK) != UTF8_CONT_VALUE)
                        {
                            // Invalid continuation byte
                            return pos;
                        }
                    }
                    pos += charLen;
                }
                return pos; // number of bytes encompassing full UTF-8 chars from start
            }

            void flushBuffer(Counter& counter)
            {
                if (m_buffer.empty())
                {
                    return;
                }

                std::size_t flushBytes = fullCharsByteCount();

                if (flushBytes == 0)
                {
                    return;
                }

                try
                {
                    std::string encoding = std::use_facet<boost::locale::info>(m_locale).encoding();
                    std::wstring wideStr = boost::locale::conv::to_utf<wchar_t>(
                        m_buffer.substr(0, flushBytes), encoding);
                    counter.multibyte += wideStr.size();
                    m_buffer.erase(0, flushBytes);
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
                if (!m_buffer.empty())
                {
                    counter.multibyte += 1;
                    m_buffer.clear();
                }
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