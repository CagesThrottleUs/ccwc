#ifndef CCWC_ALGORITHM_UNIVERSAL_INPUT_STREAM_HPP
#define CCWC_ALGORITHM_UNIVERSAL_INPUT_STREAM_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace ccwc::algorithm
{

    /**
     * @brief A class that can be used to read from a file or stdin.
     *
     * This class is responsible for reading from a file or stdin. When the scenario is file, it
     * will read the file via the kernel provided memory mapped I/O while only providing the
     * interface for a string stream.
     *
     * For smaller files, which is less than 100MB, it will read the file into memory and provide
     * the interface for a string stream.
     *
     * In case of stdin, it will read from the standard input stream but will also be processed as
     * string stream.
     */
    class UniversalInputStream
    {
      public:
        /**
         * @brief Destructor for the UniversalInputStream class.
         */
        virtual ~UniversalInputStream() = default;

        /**
         * @brief Get the logical name (e.g., filename or "<stdin>")
         */
        virtual std::string name() const = 0;

        /**
         * @brief Returns true if this stream is stdin.
         */
        virtual bool isStdin() const = 0;

        /**
         * @brief Returns the next byte from the stream.
         * For stdin this may be unavailable, so return std::nullopt.
         * std::nullopt means the end of the stream has been reached.
         */
        virtual std::optional<unsigned char> nextByte() = 0;

        /**
         * @brief Reset the stream to the beginning.
         * @return True if the stream was reset successfully.
         */
        virtual bool reset() = 0;

        /**
         * @brief Check if the stream is still valid.
         */
        virtual bool good() const = 0;
    };

    /**
     * @brief Creates a new input stream for a file.
     * @param filename The name of the file.
     * @return A new input stream for the file.
     */
    auto createInputStream(const std::string& filename) -> std::unique_ptr<UniversalInputStream>;

    /**
     * @brief Creates a new input stream for stdin.
     * @return A new input stream for stdin.
     */
    auto createInputStream() -> std::unique_ptr<UniversalInputStream>;
} // namespace ccwc::algorithm

#endif // CCWC_ALGORITHM_UNIVERSAL_INPUT_STREAM_HPP