#include "Logger.h"
#include <thread>
#include <format>
#include <iostream>

#include "../ProjectModule/ProjectModule.h"
#include "../FilesystemModule/FilesystemModule.h"

namespace Logger
{
	void Logger::log(LogVerbosity verbosity, const std::string& message, const std::string& category)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		std::string formattedMessage = std::format("[{}] [{}] {}", category.empty() ? "Unnamed" : category, toString(verbosity), message);
		std::cout << formattedMessage << std::endl;

		std::thread saveToLogFile([=]()
			{
				std::string logsPath = m_projectModule->getProjectConfig().getLogsPath();
				if (!logsPath.empty())
				{
					std::string fullPath = logsPath + "/log" + m_projectModule->getProjectConfig().getOpenTime() + ".txt";
					if (m_filesystemModule->isPathExists(fullPath))
					{
						m_filesystemModule->appendToTextFile(fullPath, formattedMessage);
					}
					else
					{
						m_filesystemModule->writeTextFile(fullPath, formattedMessage);
					}
				}

			});
		saveToLogFile.detach();

		OnMessagePushed(formattedMessage);
	}

	std::string Logger::toString(LogVerbosity verbosity) const
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

	void Logger::startup(const ModuleRegistry& registry) 
	{
		m_filesystemModule = std::dynamic_pointer_cast<FilesystemModule::FilesystemModule>(registry.getModule("FilesystemModule"));
		m_projectModule = std::dynamic_pointer_cast<ProjectModule::ProjectModule>(registry.getModule("ProjectModule"));
	}

	void Logger::shutdown() {}
}

