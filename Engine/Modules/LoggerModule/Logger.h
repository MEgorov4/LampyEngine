#pragma once
#include <string>
#include <memory>
#include <mutex>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

#include "../EventModule/Event.h"

namespace FilesystemModule
{
	class FilesystemModule;
}

namespace ProjectModule
{
	class ProjectModule;
}

namespace Logger
{
	/// <summary>
	/// Defines different log verbosity levels.
	/// </summary>
	enum class LogVerbosity : uint8_t
	{
		Verbose,  ///< Detailed debug information.
		Debug,    ///< General debugging messages.
		Info,     ///< General information messages.
		Warning,  ///< Warnings that may indicate potential issues.
		Error,    ///< Errors that indicate failures but do not stop execution.
		Fatal     ///< Critical errors that may terminate execution.
	};

	/// <summary>
	/// Singleton class for logging messages with different verbosity levels.
	/// Supports event-based logging via the OnMessagePushed event.
	/// </summary>
	class Logger : public IModule
	{
		std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
		std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;

		std::mutex m_mutex; ///< Mutex to ensure thread-safe logging.
	public:
		/// <summary>
		/// Event that triggers when a new log message is pushed.
		/// Subscribers receive the formatted log message.
		/// </summary>
		Event<std::string> OnMessagePushed;

		/// <summary>
		/// Logs a message with a specified verbosity level.
		/// </summary>
		/// <param name="verbosity">The verbosity level of the log.</param>
		/// <param name="message">The message to be logged.</param>
		void log(LogVerbosity verbosity, const std::string& message, const std::string& category = "");

		void startup(const ModuleRegistry& registry) override;
		void shutdown() override;

	private:
		/// <summary>
		/// Converts a LogVerbosity value to a string representation.
		/// </summary>
		/// <param name="verbosity">The verbosity level.</param>
		/// <returns>String representation of the verbosity level.</returns>
		std::string toString(LogVerbosity verbosity) const;
	};
}
