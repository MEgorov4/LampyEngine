#include "ProjectModule.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <portable-file-dialogs.h>
#include <nlohmann/json.hpp>
#include <boost/process.hpp>

#include <chrono>
#include "../LoggerModule/Logger.h"

#include "../FilesystemModule/FilesystemModule.h"

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

void ProjectModule::setupProjectEnvironment()
{
	LOG_INFO("ProjectModule: Run project browser");
	namespace bp = boost::process;
	
	bp::ipstream out_stream;
	bp::child projectBrowserProcess("Debug/ProjectBrowser.exe", bp::std_out > out_stream);
	
	std::string line;
	while (std::getline(out_stream, line))
	{
		LOG_INFO(line);
		m_projectConfig = ProjectConfig(line);
	}
	projectBrowserProcess.wait();

	if (projectBrowserProcess.exit_code() != 0)
	{
		std::exit(0);
	}

}
void ProjectModule::saveProjectConfig()
{
	nlohmann::json jsonData;

	jsonData["projectPath"] = m_projectConfig.getProjectPath();
	jsonData["projectName"] = m_projectConfig.getProjectName();

	jsonData["resourcesPath"] = m_projectConfig.getResourcesPath();
	jsonData["buildPath"] = m_projectConfig.getBuildPath();
	jsonData["configPath"] = m_projectConfig.getConfigPath();
	jsonData["logsPath"] = m_projectConfig.getLogsPath();

	jsonData["editorStartWorld"] = m_projectConfig.getEditorStartWorld();
	jsonData["gameStartWorld"] = m_projectConfig.getGameStartWorld();
	
	std::string projectFilePath = m_projectConfig.getProjectPath() + '/' + m_projectConfig.getProjectName() + ".lproj";
	if (FS.writeTextFile(projectFilePath, jsonData.dump()) == FResult::SUCCESS)
	{
		LOG_INFO("ProjectModule: Project settings saved.");
	}

}
