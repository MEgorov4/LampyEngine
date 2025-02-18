#include "Logger.h"
#include <thread>

#include "../ProjectModule/ProjectModule.h"
#include "../FilesystemModule/FilesystemModule.h"

 void Logger::Log(LogVerbosity verbosity, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string formattedMessage = std::format("[{}] {}", ToString(verbosity), message);
    std::cout << formattedMessage << std::endl;

    std::thread saveToLogFile([formattedMessage]()
        {
            std::string logsPath = PM.getProjectConfig().getLogsPath();
            if (!logsPath.empty())
            {
                std::string fullPath = logsPath + "/log" + PM.getProjectConfig().getOpenTime() + ".txt";
                if (FS.isPathExists(fullPath))
                {
                    FS.appendToTextFile(fullPath, formattedMessage);
                }
                else
                {
                    FS.writeTextFile(fullPath, formattedMessage);
                }
            }

        });
    saveToLogFile.detach();

    OnMessagePushed(formattedMessage);
}

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
