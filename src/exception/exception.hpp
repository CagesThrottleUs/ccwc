#ifndef CCWC_EXCEPTION_EXCEPTION_HPP
#define CCWC_EXCEPTION_EXCEPTION_HPP

#include <exception>
#include <string>

/**
 * @brief Exception namespace.
 */
namespace ccwc::exception
{
    /**
     * @brief Exception thrown when a file operation fails.
     */
    class FileOperationException : public std::exception
    {
      private:
        /**
         * @brief The error message.
         */
        std::string m_message;

      public:
        /**
         * @brief Constructor for the FileOperationException class.
         * @param message The error message.
         */
        explicit FileOperationException(std::string message) : m_message(std::move(message))
        {
        }

        /**
         * 
         */
        [[nodiscard]] auto what() const noexcept -> const char* override
        {
            return m_message.c_str();
        }
    };
} // namespace ccwc::exception

#endif // CCWC_EXCEPTION_EXCEPTION_HPP