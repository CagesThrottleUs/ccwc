#ifndef CCWC_ARGUMENT_PARSER_INPUT_OBJECTS_HPP
#define CCWC_ARGUMENT_PARSER_INPUT_OBJECTS_HPP

#include "algorithm/universal_input_stream.hpp"

#include <memory>
#include <string>

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
        std::unique_ptr<ccwc::algorithm::UniversalInputStream> mInputStream;

        /**
         * @brief The health status of the input stream.
         */
        HealthStatus mHealthStatus;
    };
} // namespace ccwc::argument_parser

#endif // CCWC_ARGUMENT_PARSER_INPUT_OBJECTS_HPP