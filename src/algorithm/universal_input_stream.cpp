#include "universal_input_stream.hpp"

#include "exception/exception.hpp"

#include <boost/iostreams/device/mapped_file.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace ccwc::algorithm
{
    namespace detail
    {
        /**
         * @brief A class that represents the standard input stream.
         */
        class StandardInputStream : public UniversalInputStream
        {
          private:
            /**
             * @brief The name of the standard input stream.
             */
            std::string m_name{"<stdin>"};

          public:
            /**
             * @brief Default constructor.
             */
            StandardInputStream() = default;

            /**
             * @brief Destructor.
             */
            ~StandardInputStream() override = default;

            /**
             * @brief Copy constructor.
             */
            StandardInputStream(const StandardInputStream& other) = default;

            /**
             * @brief Copy assignment.
             */
            auto operator=(const StandardInputStream& other) -> StandardInputStream& = default;

            /**
             * @brief Move constructor.
             */
            StandardInputStream(StandardInputStream&& other) noexcept = default;

            /**
             * @brief Move assignment.
             */
            auto operator=(StandardInputStream&& other) noexcept -> StandardInputStream& = default;

            /**
             * @brief Returns the name of the standard input stream.
             */
            [[nodiscard]] auto name() const -> std::string override
            {
                return m_name;
            }

            /**
             * @brief Returns true if the standard input stream is stdin.
             */
            [[nodiscard]] auto isStdin() const -> bool override
            {
                return true;
            }

            /**
             * @brief Returns the next byte from the standard input stream.
             */
            auto nextByte() -> std::optional<unsigned char> override
            {
                int nextByte = std::cin.get();
                if (nextByte == EOF)
                {
                    return std::nullopt;
                }
                return static_cast<unsigned char>(nextByte);
            }

            /**
             * @brief Resets the standard input stream to the beginning.
             */
            auto reset() -> bool override
            {
                return false; // stdin cannot rewind
            }

            /**
             * @brief Returns true if the standard input stream is valid.
             */
            [[nodiscard]] auto good() const -> bool override
            {
                return std::cin.good();
            }
        };
    } // namespace detail

    /**
     * @brief Creates a new standard input stream.
     * @return A new standard input stream.
     */
    auto createInputStream() -> std::unique_ptr<UniversalInputStream>
    {
        return std::make_unique<detail::StandardInputStream>();
    }

    namespace detail
    {
        /**
         * @brief Reads from a file via std::ifstream (buffered).
         */
        class BufferedFileInputStream : public UniversalInputStream
        {
          private:
            /**
             * @brief The name of the file.
             */
            std::string m_name;

            /**
             * @brief The stream to read from.
             */
            mutable std::ifstream m_stream;

          public:
            /**
             * @brief Constructor.
             * @param filename The name of the file.
             */
            explicit BufferedFileInputStream(std::string filename) : m_name(std::move(filename))
            {
                m_stream = std::ifstream(m_name, std::ios::binary);
            }

            /**
             * @brief Destructor.
             */
            ~BufferedFileInputStream() override = default;

            /**
             * @brief Copy constructor. explicitely deleted because ifstream is not copyable.
             */
            BufferedFileInputStream(const BufferedFileInputStream&) = delete;

            /**
             * @brief Copy assignment. explicitely deleted because ifstream is not copyable.
             */
            auto operator=(const BufferedFileInputStream&) -> BufferedFileInputStream& = delete;

            /**
             * @brief Move constructor.
             */
            BufferedFileInputStream(BufferedFileInputStream&&) noexcept = default;

            /**
             * @brief Move assignment.
             */
            auto operator=(BufferedFileInputStream&&) noexcept
                -> BufferedFileInputStream& = default;

            /**
             * @brief Returns the name of the file.
             */
            [[nodiscard]] auto name() const -> std::string override
            {
                return m_name;
            }

            /**
             * @brief Returns true if the stream is stdin.
             */
            [[nodiscard]] auto isStdin() const -> bool override
            {
                return false;
            }

            /**
             * @brief Returns the next byte from the stream.
             */
            [[nodiscard]] auto nextByte() -> std::optional<unsigned char> override
            {
                char nextByte{};
                if (m_stream.get(nextByte))
                {
                    return static_cast<unsigned char>(nextByte);
                }
                return std::nullopt;
            }

            /**
             * @brief Resets the stream to the beginning.
             */
            [[nodiscard]] auto reset() -> bool override
            {
                if (!m_stream.is_open())
                {
                    return false;
                }
                m_stream.clear();
                m_stream.seekg(0, std::ios::beg);
                return m_stream.good();
            }

            /**
             * @brief Returns true if the stream is valid.
             */
            [[nodiscard]] auto good() const -> bool override
            {
                return m_stream.good();
            }
        };

        /**
         * @brief An input stream backed by memory-mapped I/O.
         *
         * This class uses Boost.Iostreams' memory-mapped file device to
         * provide zero-copy access to file contents. It is highly efficient
         * for large files since the OS kernel handles paging, and only the
         * accessed portions of the file are brought into memory.
         *
         * Features:
         *   - Provides a byte-by-byte streaming interface via nextByte().
         *   - Supports reset(), which rewinds the file to the beginning.
         *   - Random access could be added later (if needed).
         *
         * Limitations:
         *   - Only valid for regular files (not stdin, pipes, or sockets).
         *   - The file must remain valid on disk while this object is alive.
         */
        class MemoryMappedFileInputStream : public UniversalInputStream
        {
          private:
            std::string                          m_name;   // Logical name (file path).
            boost::iostreams::mapped_file_source m_map;    // Boost memory-mapped file.
            std::size_t                          m_pos{0}; // Current read position.

          public:
            /**
             * @brief Constructor: opens and memory-maps the given file.
             * @param filename Path to the file to be memory-mapped.
             * @throws std::runtime_error if mapping fails.
             */
            explicit MemoryMappedFileInputStream(const std::string& filename) : m_name(filename)
            {
                m_map.open(filename);
                if (!m_map.is_open())
                {
                    throw std::runtime_error("Failed to memory-map file: " + filename);
                }
            }

            /**
             * @brief Destructor (closes the mapping).
             */
            ~MemoryMappedFileInputStream() override = default;

            /**
             * @brief Copy constructor (disabled: mapping a file twice is unsafe).
             */
            MemoryMappedFileInputStream(const MemoryMappedFileInputStream&) = delete;

            /**
             * @brief Copy assignment (disabled).
             */
            auto operator=(const MemoryMappedFileInputStream&)
                -> MemoryMappedFileInputStream& = delete;

            /**
             * @brief Move constructor.
             */
            MemoryMappedFileInputStream(MemoryMappedFileInputStream&&) = delete;

            /**
             * @brief Move assignment.
             */
            auto operator=(MemoryMappedFileInputStream&&) -> MemoryMappedFileInputStream& = delete;

            /**
             * @brief Get the logical name (file path).
             */
            [[nodiscard]] auto name() const -> std::string override
            {
                return m_name;
            }

            /**
             * @brief This is not stdin.
             */
            [[nodiscard]] auto isStdin() const -> bool override
            {
                return false;
            }

            /**
             * @brief Get the next byte from the mapped file.
             * @return std::optional<unsigned char>: the next byte,
             *         or std::nullopt if EOF.
             */
            auto nextByte() -> std::optional<unsigned char> override
            {
                if (m_pos >= m_map.size())
                {
                    return std::nullopt;
                }
                // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return static_cast<unsigned char>(m_map.data()[m_pos++]);
                // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }

            /**
             * @brief Reset the stream to the beginning of the file.
             * @return True if reset succeeded.
             */
            auto reset() -> bool override
            {
                m_pos = 0;
                return true;
            }

            /**
             * @brief Check if the mapping is still valid.
             */
            [[nodiscard]] auto good() const -> bool override
            {
                return m_map.is_open();
            }

            // ===== Additional helpers =====

            /**
             * @brief Get the total size of the mapped file.
             */
            [[nodiscard]] auto size() const noexcept -> std::size_t
            {
                return m_map.size();
            }
        };

        constexpr std::size_t MAX_MEMORY_MAPPED_FILE_SIZE =
            static_cast<std::size_t>(100 * 1024 * 1024); // 100MB
    } // namespace detail

    auto createInputStream(const std::string& filename) -> std::unique_ptr<UniversalInputStream>
    {
        std::size_t fileSize{0};

        try
        {
            fileSize = std::filesystem::file_size(filename);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            throw ccwc::exception::FileOperationException(e.what());
        }

        if (fileSize < detail::MAX_MEMORY_MAPPED_FILE_SIZE)
        {
            return std::make_unique<detail::BufferedFileInputStream>(filename);
        }

        return std::make_unique<detail::MemoryMappedFileInputStream>(filename);
    }
} // namespace ccwc::algorithm