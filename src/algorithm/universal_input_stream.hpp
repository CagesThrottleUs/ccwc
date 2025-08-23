#ifndef CCWC_ALGORITHM_UNIVERSAL_INPUT_STREAM_HPP
#define CCWC_ALGORITHM_UNIVERSAL_INPUT_STREAM_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <variant>

namespace ccwc::algorithm
{

    /**
     * @brief A class that can be used to read from a file or stdin.
     */
    class UniversalInputStream
    {
      private:
        /**
         * @brief The stream to read from.
         */
        std::variant<std::ifstream, std::istream*> m_stream;

        /**
         * @brief The filename of the stream.
         */
        std::string m_filename;

      public:
        /**
         * @brief Constructor for file input.
         * @param filename The filename of the file to read from.
         */
        explicit UniversalInputStream(const std::string& filename) : m_filename(filename)
        {

            std::ifstream fileStream(filename, std::ios::binary);
            if (!fileStream.is_open())
            {
                throw std::runtime_error("Cannot open file: " + filename);
            }
            m_stream = std::move(fileStream);
        }

        /**
         * @brief Constructor for stdin.
         */
        UniversalInputStream() : m_stream(&std::cin), m_filename("<stdin>")
        {
        }

        /**
         * @brief Get the underlying stream non-const version.
         * @return The underlying stream.
         */
        auto stream() -> std::istream&
        {
            return std::visit(
                [](auto&& arg) -> std::istream&
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::ifstream>)
                    {
                        return arg;
                    }
                    else
                    {
                        return *arg; // std::istream*
                    }
                },
                m_stream);
        }

        /**
         * @brief Get the underlying stream const version.
         * @return The underlying stream.
         */
        auto stream() const -> const std::istream&
        {
            return std::visit(
                [](auto&& arg) -> const std::istream&
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::ifstream>)
                    {
                        return arg;
                    }
                    else
                    {
                        return *arg; // std::istream*
                    }
                },
                m_stream);
        }

        /**
         * @brief Get the name of the stream.
         * @return The name of the stream.
         */
        auto name() const -> const std::string&
        {
            return m_filename;
        }

        /**
         * @brief Check if the stream is stdin.
         * @return True if the stream is stdin, false otherwise.
         */
        auto isStdin() const -> bool
        {
            return std::holds_alternative<std::istream*>(m_stream);
        }

        /**
         * @brief Check if the stream is good.
         * @return True if the stream is good, false otherwise.
         */
        auto good() const -> bool
        {
            return stream().good();
        }

        /**
         * @brief Check if the stream is good.
         * @return True if the stream is good, false otherwise.
         */
        explicit operator bool() const
        {
            return good();
        }
    };
} // namespace ccwc::algorithm

#endif // CCWC_ALGORITHM_UNIVERSAL_INPUT_STREAM_HPP