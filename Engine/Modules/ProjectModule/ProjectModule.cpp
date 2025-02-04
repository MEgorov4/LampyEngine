#include "ProjectModule.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <portable-file-dialogs.h>
#include <nlohmann/json.hpp>
#include <boost/process.hpp>
#include "../LoggerModule/Logger.h"

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
}

void ProjectModule::setupProjectEnvironment()
{	
	LOG_INFO("ProjectModule: Run project browser");
	namespace bp = boost::process;
	
	bp::ipstream out_stream;
	bp::child projectBrowserProcess("../../build/bin/debug/ProjectBrowser.exe", bp::std_out > out_stream);
	
	std::string line;
	while (std::getline(out_stream, line))
	{
		LOG_INFO(line);
		m_projectConfig = ProjectConfig(line);
	}
	projectBrowserProcess.wait();

}
void ProjectModule::saveProjectConfig()
{
	//nlohmann::json jsonData;

	//jsonData["projectPath"] = m_projectConfig.getProjectPath();
	//jsonData["projectName"] = m_projectConfig.getProjectName();

	//jsonData["resourcesPath"] = m_projectConfig.getResourcesPath();
	//jsonData["buildPath"] = m_projectConfig.getBuildPath();
	//jsonData["configPath"] = m_projectConfig.getConfigPath();
	//jsonData["logsPath"] = m_projectConfig.getLogsPath();

	//jsonData["editorStartWorld"] = m_projectConfig.getEditorStartWorld();
	//jsonData["gameStartWorld"] = m_projectConfig.getGameStartWorld();

	//std::ofstream outFile(m_projectConfig.getProjectPath());
	//if (outFile.is_open())
	//{
	//	outFile.clear();
	//	outFile << jsonData;
	//	outFile.close();
	//}
}
