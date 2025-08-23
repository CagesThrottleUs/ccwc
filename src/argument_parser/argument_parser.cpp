#include "argument_parser.hpp"

#include "exception/exception.hpp"

#include <span>
#include <string_view>

// PRIVATE INTERFACE

namespace ccwc::argument_parser
{

    auto Arguments::addFormattingOptions(
        ccwc::output_format_options::OutputFormatOptions formatOption) -> void
    {
        m_output_formatter.addOption(formatOption);
    }

    auto Arguments::addInputFile(const std::string& filename) -> void
    {
        InputDataObject                       inputDataObject;
        ccwc::algorithm::UniversalInputStream inputStream;
        HealthStatus                          healthStatus;
        try
        {
            inputStream                   = ccwc::algorithm::UniversalInputStream(filename);
            inputDataObject.mInputStream  = std::move(inputStream);
            inputDataObject.mHealthStatus = HealthStatus(true, "");
            m_input_data_objects.emplace_back(std::move(inputDataObject));
        }
        catch (const ccwc::exception::FileOperationException& e)
        {
            inputStream                   = ccwc::algorithm::UniversalInputStream();
            inputDataObject.mInputStream  = std::move(inputStream);
            inputDataObject.mHealthStatus = HealthStatus(false, e.what());
            m_input_data_objects.emplace_back(std::move(inputDataObject));
        }
    }

    auto Arguments::addStdin() -> void
    {
        if (m_input_data_objects.empty())
        {
            InputDataObject inputDataObject;
            inputDataObject.mInputStream  = ccwc::algorithm::UniversalInputStream();
            inputDataObject.mHealthStatus = HealthStatus(true, "");
            m_input_data_objects.emplace_back(std::move(inputDataObject));
        }
    }

    auto Arguments::inputDataObjects() const -> const std::vector<InputDataObject>&
    {
        return m_input_data_objects;
    }

} // namespace ccwc::argument_parser

// PUBLIC EXPOSED INTERFACE

namespace ccwc
{

    namespace detail
    {

        auto processOption(std::string_view arg, ccwc::argument_parser::Arguments& args) -> void
        {
            if (arg == "-l")
            {
                args.addFormattingOptions(
                    ccwc::output_format_options::OutputFormatOptions::FORMAT_LINES);
            }
            else if (arg == "-w")
            {
                args.addFormattingOptions(
                    ccwc::output_format_options::OutputFormatOptions::FORMAT_WORDS);
            }
            else if (arg == "-c")
            {
                args.addFormattingOptions(
                    ccwc::output_format_options::OutputFormatOptions::FORMAT_BYTES);
            }
            else if (arg == "-m")
            {
                args.addFormattingOptions(
                    ccwc::output_format_options::OutputFormatOptions::FORMAT_MULTIBYTE);
            }
        }
    } // namespace detail

    auto parseArguments(int argc, char** argv) -> ccwc::argument_parser::Arguments
    {
        ccwc::argument_parser::Arguments args;

        std::span<char*> safeArgs(argv, static_cast<std::size_t>(argc));

        for (char* arg : safeArgs)
        {
            std::string_view argView(arg);

            if (argView.starts_with("-"))
            {
                detail::processOption(argView, args);
            }
            else
            {
                args.addInputFile(std::string(argView));
            }
        }

        args.addStdin();

        return args;
    }

} // namespace ccwc