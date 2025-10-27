#include "ProjectModule.h"

#include <portable-file-dialogs.h>
#include <nlohmann/json.hpp>
#include <boost/process.hpp>

namespace ProjectModule
{
	ProjectConfig::ProjectConfig(std::string data)
	{
		nlohmann::json jsonData = nlohmann::json::parse(data);

		projectPath = jsonData["projectPath"];
		projectName = jsonData["projectName"];

		resourcesPath = jsonData["resourcesPath"];
		configPath = jsonData["configPath"];
		logsPath = jsonData["logsPath"];
		buildPath = jsonData["buildPath"];

		editorStartWorld = jsonData["editorStartWorld"];
		gameStartWorld = jsonData["gameStartWorld"];

		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);

		std::tm localTime;

#ifdef _WIN32
		localtime_s(&localTime, &now_c);
#else 
		localtime_r(&now_c, &localTime);
#endif

		std::ostringstream oss;

		oss << std::put_time(&localTime, "[%Y-%m-%d]-[%H-%M-%S]");

		openTime = oss.str();
	}

	std::string ProjectConfig::getOpenTime() const
	{ return openTime; }

	std::string ProjectConfig::getProjectPath() const
	{ return projectPath; }

	std::string ProjectConfig::getProjectName() const
	{ return projectName; }

	std::string ProjectConfig::getResourcesPath() const
	{ return resourcesPath; }

	std::string ProjectConfig::getConfigPath() const
	{ return configPath; }

	std::string ProjectConfig::getLogsPath() const
	{ return configPath; }

	std::string ProjectConfig::getBuildPath() const
	{ return buildPath; }

	std::string ProjectConfig::getEditorStartWorld() const
	{ return editorStartWorld; }

	std::string ProjectConfig::getGameStartWorld() const
	{ return gameStartWorld; }

	void ProjectConfig::setEditorStartWorld(const std::string& path)
	{ editorStartWorld = path; }

	ProjectConfig& ProjectModule::getProjectConfig()
	{ return m_projectConfig; }
	
	void ProjectModule::startup()
	{
		LT_LOG(LogVerbosity::Info, "ProjectModule", "Startup");
		setupProjectEnvironment();
	}

	void ProjectModule::shutdown()
	{
		LT_LOG(LogVerbosity::Info, "ProjectModule", "Shutdown");
		saveProjectConfig();
	}

	void ProjectModule::setupProjectEnvironment()
	{
		namespace bp = boost::process;

		LT_LOG(LogVerbosity::Info, "ProjectModule", "Load project config");

		bp::ipstream out_stream;
		bp::child projectBrowserProcess("Debug/ProjectBrowser.exe", bp::std_out > out_stream);

		std::string line;
		while (std::getline(out_stream, line))
		{
			LT_LOG(LogVerbosity::Info, "ProjectModule", "Loaded project info: " + line);
			m_projectConfig = ProjectConfig(line);
		}

		projectBrowserProcess.wait();

		if (projectBrowserProcess.exit_code() != 0)
		{
			LT_LOG(LogVerbosity::Error, "ProjectModule", "Project browser exited with error, shutting down.");
			std::exit(0);
		}
	}

	void ProjectModule::saveProjectConfig()
	{
		LT_LOG(LogVerbosity::Debug, "ProjectModule", "Save project config");

		nlohmann::json jsonData;
		jsonData["projectPath"] = m_projectConfig.getProjectPath();
		jsonData["projectName"] = m_projectConfig.getProjectName();
		jsonData["resourcesPath"] = m_projectConfig.getResourcesPath();
		jsonData["buildPath"] = m_projectConfig.getBuildPath();
		jsonData["configPath"] = m_projectConfig.getConfigPath();
		jsonData["logsPath"] = m_projectConfig.getLogsPath();
		jsonData["editorStartWorld"] = m_projectConfig.getEditorStartWorld();
		jsonData["gameStartWorld"] = m_projectConfig.getGameStartWorld();

		std::string projectFilePath =
			m_projectConfig.getProjectPath() + '/' + m_projectConfig.getProjectName() + ".lproj";
		
			
		if (Fs::writeTextFile(projectFilePath, jsonData.dump()) ==
			FsResult::Success)
		{
			LT_LOG(LogVerbosity::Info, "ProjectModule", "Project config saved successfully.");
		}
		else
		{
			LT_LOG(LogVerbosity::Error, "ProjectModule", "Failed to save project config.");
		}
	}

}
