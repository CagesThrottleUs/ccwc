#ifndef CCWC_ARGUMENT_PARSER_HPP
#define CCWC_ARGUMENT_PARSER_HPP

#include "argument_parser/input_objects.hpp"
#include "output_formatter/output_formatter.hpp"

#include <vector>

namespace ccwc::argument_parser
{

    /**
     * @brief Arguments class that will be used to store the arguments passed to the program.
     */
    class Arguments
    {

      private:
        /**
         * @brief The output formatter to use.
         */
        ccwc::output_formatter::OutputFormatter m_output_formatter;

        /**
         * @brief The health status of the arguments.
         */
        std::vector<InputDataObject> m_input_data_objects;

      public:
        /**
         * @brief Constructor for the Arguments class.
         */
        Arguments() = default;

        // copy constructor, move constructor, copy assignment operator, move assignment operator
        Arguments(const Arguments&)                    = delete;
        Arguments(Arguments&&)                         = default;
        auto operator=(const Arguments&) -> Arguments& = delete;
        auto operator=(Arguments&&) -> Arguments&      = default;

        /**
         * @brief Destructor for the Arguments class.
         */
        ~Arguments() = default;

        /**
         * @brief add formatting options to the output formatter.
         */
        auto addFormattingOptions(ccwc::output_format_options::OutputFormatOptions formatOption)
            -> void;

        /**
         * @brief Add an input file to the arguments.
         */
        auto addInputFile(const std::string& filename) -> void;

        /**
         * @brief Add stdin to the arguments.
         */
        auto addStdin() -> void;

        /**
         * @brief Get the input data objects.
         */
        [[nodiscard]] auto inputDataObjects() const -> const std::vector<InputDataObject>&;

        /**
         * @brief Format the output.
         */
        auto formatOutput(const std::vector<ccwc::algorithm::Counter>& counters) const -> void;

        /**
         * @brief Normalize the formatting options.
         */
        auto normalizeFormattingOptions() -> void;
    };

} // namespace ccwc::argument_parser

namespace ccwc
{
    auto parseArguments(int argc, char** argv) -> ccwc::argument_parser::Arguments;
} // namespace ccwc

#endif // CCWC_ARGUMENT_PARSER_HPP