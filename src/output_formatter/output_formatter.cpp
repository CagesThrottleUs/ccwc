#include "output_formatter.hpp"

namespace ccwc::output_formatter
{

    auto OutputFormatter::addOption(ccwc::output_format_options::OutputFormatOptions option) -> void
    {
        this->m_format_options.insert(option);
    }

    auto OutputFormatter::addCounter(const ccwc::algorithm::Counter& counter) -> void
    {
        this->m_total_counter += counter;
        this->m_outputs_to_format.push_back(counter);
    }

    auto OutputFormatter::formatFile() const -> std::string
    {
        (void) this->m_outputs_to_format;
        return "TODO: Format the file";
    }
} // namespace ccwc::output_formatter