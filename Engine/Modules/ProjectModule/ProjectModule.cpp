#include "ProjectModule.h"
#include <fstream>
#include <portable-file-dialogs.h>
#include <nlohmann/json.hpp>

#include "../LoggerModule/Logger.h"

ProjectModule::ProjectModule()
{
}

ProjectModule::~ProjectModule()
{
}

void ProjectModule::chooseOrCreateProject()
{
	pfd::button result = pfd::message("Project Selection", "Do you want to open an existing project?\n(Yes - open project file, No - create new project)",pfd::choice::yes_no).result();

	if (result == pfd::button::yes)
	{
		openExistingProject();
		return;
	}
	
	createNewProject();
}

void ProjectModule::openExistingProject()
{
	std::vector<std::string> paths = pfd::open_file("Select project file", "", { "Project Files", "*.lproj" }).result();

	if (paths.empty())
	{
		LOG_ERROR("ProjectModule: Can`t open project file");
		chooseOrCreateProject();
		return;
	}
	std::fstream projecFile = std::fstream(paths[0]);
	
	nlohmann::json jsonData = nlohmann::json::parse(projecFile);
	if (jsonData.empty())
	{
		LOG_ERROR("ProjectModule: Invalid project file data");
		chooseOrCreateProject();
		return;
	}

	m_projectConfig.projectPath = jsonData["projectPath"];
	m_projectConfig.projectName = jsonData["projectName"];

	m_projectConfig.resourcesPath = jsonData["resourcesPath"];
	m_projectConfig.buildPath = jsonData["buildPath"];
	m_projectConfig.configPath =  jsonData["configPath"];
	m_projectConfig.logsPath = jsonData["logsPath"];

	m_projectConfig.editorStartWorld = jsonData["gameStartWorld"];
	m_projectConfig.gameStartWorld = jsonData["gameStartWorld"];
}

void ProjectModule::createNewProject()
{
	std::string folderPath = pfd::select_folder("Select a folder for the new project").result();

	if (folderPath.empty())
	{
		chooseOrCreateProject();
		return;
	}

	if (!std::filesystem::exists(folderPath))
	{
		std::filesystem::create_directory(folderPath);
	}
	
	std::filesystem::create_directory(folderPath + "/Resources");
	std::filesystem::create_directory(folderPath + "/Saved");
	std::filesystem::create_directory(folderPath + "/Config");
	std::filesystem::create_directory(folderPath + "/Build");

	nlohmann::json jsonData;

	m_projectConfig.projectPath = folderPath + "/project.lproj";
	m_projectConfig.projectName = "project";
	jsonData["projectPath"] = m_projectConfig.projectPath;
	jsonData["projectName"] = m_projectConfig.projectName;

	m_projectConfig.resourcesPath = folderPath + "/Resources";
	m_projectConfig.buildPath = folderPath + "/Build";
	m_projectConfig.configPath = folderPath + "/Config";
	m_projectConfig.logsPath = folderPath + "/Build";

	jsonData["resourcesPath"] = m_projectConfig.resourcesPath;
	jsonData["buildPath"] = m_projectConfig.buildPath;
	jsonData["configPath"] = m_projectConfig.configPath;
	jsonData["logsPath"] = m_projectConfig.logsPath;
	
	m_projectConfig.editorStartWorld = "default";
	m_projectConfig.gameStartWorld = "default";
	jsonData["editorStartWorld"] = m_projectConfig.editorStartWorld;
	jsonData["gameStartWorld"] = m_projectConfig.gameStartWorld;

	std::ofstream outFile(folderPath + "/project.lproj");
	if (outFile.is_open())
	{
		outFile << jsonData;
		outFile.close();
	}
}

void ProjectModule::saveProjectConfig()
{
	nlohmann::json jsonData;

	jsonData["projectPath"] = m_projectConfig.projectPath;
	jsonData["projectName"] = m_projectConfig.projectName;

	jsonData["resourcesPath"] = m_projectConfig.resourcesPath;
	jsonData["buildPath"] = m_projectConfig.buildPath;
	jsonData["configPath"] = m_projectConfig.configPath;
	jsonData["logsPath"] = m_projectConfig.logsPath;

	jsonData["editorStartWorld"] = m_projectConfig.editorStartWorld;
	jsonData["gameStartWorld"] = m_projectConfig.gameStartWorld;

	std::ofstream outFile(m_projectConfig.projectPath);
	if (outFile.is_open())
	{
		outFile.clear();
		outFile << jsonData;
		outFile.close();
	}
}
