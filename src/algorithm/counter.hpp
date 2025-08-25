#ifndef CCWC_COUNTER_HPP
#define CCWC_COUNTER_HPP

#include <cstddef>

namespace ccwc::algorithm
{

    /**
     * @brief Counter struct that will be used to count the different types of characters in a file.
     */
    struct Counter
    {
      public:
        std::size_t bytes{0};
        std::size_t words{0};
        std::size_t lines{0};
        std::size_t multibyte{0};

        /**
         * @brief Constructor for the Counter struct.
         */
        Counter() = default;

        /**
         * @brief Operator to add two Counter structs together.
         */
        auto operator+=(const Counter& other) -> Counter&
        {
            this->bytes += other.bytes;
            this->words += other.words;
            this->lines += other.lines;
            this->multibyte += other.multibyte;

            return *this;
        }
    };

} // namespace ccwc::algorithm

#endif // CCWC_COUNTER_HPP