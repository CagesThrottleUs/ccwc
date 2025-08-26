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
        explicit LinesFormatHandler(std::size_t max_length, bool enabled)
            : FormatHandler(max_length, enabled)
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
        explicit WordsFormatHandler(std::size_t max_length, bool enabled)
            : FormatHandler(max_length, enabled)
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
        explicit MultibyteFormatHandler(std::size_t max_length, bool enabled)
            : FormatHandler(max_length, enabled)
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
        explicit BytesFormatHandler(std::size_t max_length, bool enabled)
            : FormatHandler(max_length, enabled)
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

    auto OutputFormatter::buildFormatChain(std::size_t max_len_of_num) const
        -> std::unique_ptr<ccwc::output_format_options::FormatHandler>
    {
        auto line = std::make_unique<ccwc::output_format_options::LinesFormatHandler>(
            max_len_of_num,
            IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_LINES));

        // clang-format off
        line
            ->setNext(
                std::make_unique<ccwc::output_format_options::WordsFormatHandler>(
                    max_len_of_num,
                    IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_WORDS)
                )
            )
            ->setNext(
                std::make_unique<ccwc::output_format_options::MultibyteFormatHandler>(
                    max_len_of_num,
                    IsOptionEnabled(ccwc::output_format_options::OutputFormatOptions::FORMAT_MULTIBYTE)
                )
            )
            ->setNext(
                std::make_unique<ccwc::output_format_options::BytesFormatHandler>(
                    max_len_of_num,
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
        ccwc::algorithm::Counter total_counter{};

        for (const auto& counter : counters)
        {
            total_counter += counter;
        }

        std::size_t max_spaces{std::to_string(total_counter.bytes).length()};

        auto format_chain = this->buildFormatChain(max_spaces);

        std::string output;
        bool        found_bad{false};
        for (std::size_t i = 0; i < counters.size(); ++i)
        {

            if (!inputDataObjects[i].mHealthStatus.mIsHealthy)
            {
                found_bad = true;
                output += inputDataObjects[i].mHealthStatus.mErrorMessage + "\n";
                break;
            }

            output = format_chain->doHandle(output, counters[i]);

            if (!inputDataObjects[i].mInputStream->isStdin())
            {
                output += " " + inputDataObjects[i].mInputStream->name();
            }
            output += "\n";
        }

        if (found_bad)
        {
            return output;
        }

        // else process total counter
        if (inputDataObjects.size() > static_cast<std::size_t>(1))
        {
            output = format_chain->doHandle(output, total_counter);
            output += "\n";
        }
        return output;
    }

    auto OutputFormatter::normalizeFormattingOptions() -> void
    {
        if (m_format_options.empty())
        {
            m_format_options.insert(ccwc::output_format_options::OutputFormatOptions::FORMAT_LINES);
            m_format_options.insert(ccwc::output_format_options::OutputFormatOptions::FORMAT_WORDS);
            m_format_options.insert(ccwc::output_format_options::OutputFormatOptions::FORMAT_BYTES);
        }
    }
} // namespace ccwc::output_formatter