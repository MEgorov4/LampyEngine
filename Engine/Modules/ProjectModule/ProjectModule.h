#pragma once 

#include "../LoggerModule/Logger.h"

struct ProjectConfig
{
	std::string projectPath;
	std::string projectName;

	std::string resourcesPath;
	std::string configPath;
	std::string logsPath;
	std::string buildPath;

	std::string editorStartWorld;
	std::string gameStartWorld;
};

class ProjectModule
{
	ProjectConfig m_projectConfig;

public:
	ProjectModule();
	~ProjectModule();

	static ProjectModule& getInstance()
	{
		static ProjectModule ProjectModule;
		return ProjectModule;
	}

	void startUp()
	{
        LOG_INFO("ProjectModule: Startup");
		chooseOrCreateProject();
	}
	
	ProjectConfig& getProjectConfig() { return m_projectConfig; }
	void shutDown()
	{
        LOG_INFO("ProjectModule: Shut down");
		saveProjectConfig();
	}

private:
	void chooseOrCreateProject();
	void openExistingProject();
	void createNewProject();
	void saveProjectConfig();
};
