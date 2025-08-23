#ifndef CCWC_ARGUMENT_PARSER_HPP
#define CCWC_ARGUMENT_PARSER_HPP

#include "algorithm/universal_input_stream.hpp"
#include "output_formatter/output_formatter.hpp"

#include <string>
#include <vector>

namespace ccwc::argument_parser
{

    /**
     * @brief Health status of the arguments.
     */
    struct HealthStatus
    {
        /**
         * @brief Whether the arguments are healthy.
         */
        bool mIsHealthy{true};

        /**
         * @brief The error message if the arguments are not healthy.
         */
        std::string mErrorMessage;

        /**
         * @brief Constructor for the HealthStatus class.
         */
        HealthStatus() = default;

        /**
         * @brief Constructor for the HealthStatus class.
         * @param isHealthy Whether the arguments are healthy.
         * @param errorMessage The error message if the arguments are not healthy.
         */
        HealthStatus(bool isHealthy, std::string errorMessage)
            : mIsHealthy(isHealthy), mErrorMessage(std::move(errorMessage))
        {
        }
    };

    /**
     * @brief Input data object.
     */
    struct InputDataObject
    {
        /**
         * @brief The input stream.
         */
        ccwc::algorithm::UniversalInputStream mInputStream;

        /**
         * @brief The health status of the input stream.
         */
        HealthStatus mHealthStatus;
    };

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
    };

} // namespace ccwc::argument_parser

namespace ccwc
{
    auto parseArguments(int argc, char** argv) -> ccwc::argument_parser::Arguments;
} // namespace ccwc

#endif // CCWC_ARGUMENT_PARSER_HPP