#ifndef CCWC_OUTPUT_FORMATTER_HPP
#define CCWC_OUTPUT_FORMATTER_HPP

#include "algorithm/counter.hpp"
#include "argument_parser/input_objects.hpp"

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace ccwc::output_format_options
{

    /**
     * @brief Enum class that will be used to store the different options for the output formatter.
     *
     * Here the order of enums matter as we will be using it to format the output - where first we
     * sort on lines, then on words, then on multibyte, then on bytes.
     */
    enum class OutputFormatOptions : std::uint8_t
    {
        FORMAT_LINES,
        FORMAT_WORDS,
        FORMAT_MULTIBYTE,
        FORMAT_BYTES,
    };

    /**
     * @brief Abstract class that will be used to handle the different options for the output
     * formatter.
     */
    class FormatHandler
    {
      private:
        /**
         * @brief The next handler in the chain.
         */
        std::unique_ptr<FormatHandler> m_next{nullptr};

        /**
         * @brief The maximum length of the number.
         */
        std::size_t m_max_len_of_num;

        /**
         * @brief Whether the handler is enabled.
         */
        bool m_enabled{true};

      protected:
        /**
         * @brief Handle the counter and return the output.
         *
         * @param counter The counter to handle.
         * @return The output string.
         */
        virtual auto handle(const ccwc::algorithm::Counter& counter) -> std::string = 0;

      public:
        /**
         * @brief Constructor for the FormatHandler class.
         */
        FormatHandler(std::size_t max_len_of_num)
            : m_next(nullptr), m_max_len_of_num(max_len_of_num), m_enabled(true)
        {
        }

        FormatHandler(std::size_t max_len_of_num, bool enabled)
            : m_next(nullptr), m_max_len_of_num(max_len_of_num), m_enabled(enabled)
        {
        }

        /**
         * @brief Disable copy operations (not suitable for chain of responsibility)
         */
        FormatHandler(const FormatHandler&)                    = delete;
        auto operator=(const FormatHandler&) -> FormatHandler& = delete;

        /**
         * @brief Enable move operations (useful for transferring ownership)
         */
        FormatHandler(FormatHandler&&)                    = default;
        auto operator=(FormatHandler&&) -> FormatHandler& = default;

        /**
         * @brief Set the next handler in the chain.
         *
         * You should use this as a chain for example:
         *
         * ```
         * auto handler = std::make_unique<LinesHandler>();
         * handler->setNext(std::make_unique<WordsHandler>())
         *        ->setNext(std::make_unique<MultibyteHandler>())
         *        ->setNext(std::make_unique<BytesHandler>());
         * ```
         *
         * @param handler The next handler in the chain.
         * @return Raw pointer to the next handler for chaining.
         */
        auto setNext(std::unique_ptr<FormatHandler> handler) -> FormatHandler*
        {
            FormatHandler* result = handler.get();
            m_next                = std::move(handler);
            return result;
        }

        /**
         * @brief Handle the counter and return the output.
         *
         * @param output The output string.
         * @param counter The counter to handle.
         * @return The updated output string.
         */
        auto doHandle(std::string& output, const ccwc::algorithm::Counter& counter) -> std::string
        {
            if (m_enabled)
            {
                std::string handle_string = this->handle(counter);
                std::size_t total_spaces_to_add =
                    1 + (m_max_len_of_num + 1) - handle_string.length();
                std::string normalized_handle_string =
                    std::string(total_spaces_to_add, ' ') + handle_string;

                output += normalized_handle_string;
            }
            if (m_next != nullptr)
            {
                return m_next->doHandle(output, counter);
            }
            return output;
        }

        /**
         * @brief Destructor - automatically handles cleanup via unique_ptr
         */
        virtual ~FormatHandler() = default;
    };

} // namespace ccwc::output_format_options

namespace ccwc::output_formatter
{

    /**
     * @brief OutputFormatter class that will be used to format the output of the program.
     */
    class OutputFormatter
    {

      private:
        /**
         * @brief The format options to use.
         */
        std::set<ccwc::output_format_options::OutputFormatOptions> m_format_options;

        auto buildFormatChain(std::size_t maxLenOfNum) const
            -> std::unique_ptr<ccwc::output_format_options::FormatHandler>;

        auto IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions option) const -> bool
        {
            return m_format_options.contains(option);
        }

      public:
        /**
         * @brief Constructor for the OutputFormatter class.
         */
        OutputFormatter() : m_format_options({})
        {
        }

        /**
         * @brief Add an option to the format options.
         *
         * @param option The option to add.
         */
        auto addOption(ccwc::output_format_options::OutputFormatOptions option) -> void;

        /**
         * @brief Format the file.
         *
         * @return The formatted output.
         */
        [[nodiscard]] auto formatFile(
            const std::vector<ccwc::algorithm::Counter>&               counters,
            const std::vector<ccwc::argument_parser::InputDataObject>& inputDataObjects) const
            -> std::string;

        /**
         * @brief Normalize the formatting options.
         */
        auto normalizeFormattingOptions() -> void;
    };

} // namespace ccwc::output_formatter

#endif // CCWC_OUTPUT_FORMATTER_HPP