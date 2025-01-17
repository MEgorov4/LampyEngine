#include "Logger.h"

std::string Logger::ToString(LogVerbosity verbosity) const
{
    switch (verbosity)
    {
        case LogVerbosity::Verbose:  return "Verbose";
        case LogVerbosity::Debug:    return "Debug";
        case LogVerbosity::Info:     return "Info";
        case LogVerbosity::Warning:  return "Warning";
        case LogVerbosity::Error:    return "Error";
        case LogVerbosity::Fatal:    return "Fatal";
    }
    return "Unknown";
}
