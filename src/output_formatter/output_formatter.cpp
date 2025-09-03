#include "output_formatter.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>

namespace ccwc::output_format_options
{
    class LinesFormatHandler : public FormatHandler
    {

      protected:
        auto handle(const ccwc::algorithm::Counter& counter) -> std::string override
        {
            return std::to_string(counter.lines);
        }

      public:
        explicit LinesFormatHandler(std::size_t maxLength, bool enabled)
            : FormatHandler(maxLength, enabled)
        {
        }
    };

    class WordsFormatHandler : public FormatHandler
    {
      protected:
        auto handle(const ccwc::algorithm::Counter& counter) -> std::string override
        {
            return std::to_string(counter.words);
        }

      public:
        explicit WordsFormatHandler(std::size_t maxLength, bool enabled)
            : FormatHandler(maxLength, enabled)
        {
        }
    };

    class MultibyteFormatHandler : public FormatHandler
    {
      protected:
        auto handle(const ccwc::algorithm::Counter& counter) -> std::string override
        {
            return std::to_string(counter.multibyte);
        }

      public:
        explicit MultibyteFormatHandler(std::size_t maxLength, bool enabled)
            : FormatHandler(maxLength, enabled)
        {
        }
    };

    class BytesFormatHandler : public FormatHandler
    {
      protected:
        auto handle(const ccwc::algorithm::Counter& counter) -> std::string override
        {
            return std::to_string(counter.bytes);
        }

      public:
        explicit BytesFormatHandler(std::size_t maxLength, bool enabled)
            : FormatHandler(maxLength, enabled)
        {
        }
    };
} // namespace ccwc::output_format_options

namespace ccwc::output_formatter
{

    auto OutputFormatter::addOption(ccwc::output_format_options::OutputFormatOptions option) -> void
    {
        this->m_format_options.insert(option);
    }

    auto OutputFormatter::buildFormatChain(std::size_t maxLenOfNum) const
        -> std::unique_ptr<ccwc::output_format_options::FormatHandler>
    {
        auto line = std::make_unique<ccwc::output_format_options::LinesFormatHandler>(
            maxLenOfNum,
            IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_LINES));

        // clang-format off
        line
            ->setNext(
                std::make_unique<ccwc::output_format_options::WordsFormatHandler>(
                    maxLenOfNum,
                    IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_WORDS)
                )
            )
            ->setNext(
                std::make_unique<ccwc::output_format_options::MultibyteFormatHandler>(
                    maxLenOfNum,
                    IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_MULTIBYTE)
                )
            )
            ->setNext(
                std::make_unique<ccwc::output_format_options::BytesFormatHandler>(
                    maxLenOfNum,
                    IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_BYTES)
                )
            );
        // clang-format on

        return line;
    }

    auto OutputFormatter::formatFile(
        const std::vector<ccwc::algorithm::Counter>&               counters,
        const std::vector<ccwc::argument_parser::InputDataObject>& inputDataObjects) const
        -> std::string
    {
        ccwc::algorithm::Counter totalCounter{};

        for (const auto& counter : counters)
        {
            totalCounter += counter;
        }

        std::size_t maxSpaces{std::to_string(totalCounter.bytes).length()};

        auto formatChain = this->buildFormatChain(maxSpaces);

        std::string output;
        bool        foundBad{false};
        for (std::size_t i = 0; i < counters.size(); ++i)
        {

            if (!inputDataObjects[i].mHealthStatus.mIsHealthy)
            {
                foundBad = true;
                output += inputDataObjects[i].mHealthStatus.mErrorMessage + "\n";
                break;
            }

            output = formatChain->doHandle(output, counters[i]);

            if (!inputDataObjects[i].mInputStream->isStdin())
            {
                output += " " + inputDataObjects[i].mInputStream->name();
            }
            output += "\n";
        }

        if (foundBad)
        {
            return output;
        }

        // else process total counter
        if (inputDataObjects.size() > static_cast<std::size_t>(1))
        {
            output = formatChain->doHandle(output, totalCounter);
            output += "\n";
        }
        return output;
    }

    auto OutputFormatter::normalizeFormattingOptions() -> void
    {
        if (this->m_format_options.empty())
        {
            this->m_format_options.insert(
                ccwc::output_format_options::OutputFormatOptions::FORMAT_LINES);
            this->m_format_options.insert(
                ccwc::output_format_options::OutputFormatOptions::FORMAT_WORDS);
            this->m_format_options.insert(
                ccwc::output_format_options::OutputFormatOptions::FORMAT_BYTES);
        }
    }
} // namespace ccwc::output_formatter