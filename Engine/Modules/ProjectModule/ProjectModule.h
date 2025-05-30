#pragma once 

#include "../LoggerModule/Logger.h"
#include <string>

/// <summary>
/// Represents the configuration settings of a project, including file paths and world settings.
/// </summary>
struct ProjectConfig
{
private:
    std::string openTime; ///< Time when project was opened
    std::string projectPath; ///< Path to the project directory.
    std::string projectName; ///< Name of the project.

    std::string resourcesPath; ///< Path to the project's resources directory.
    std::string configPath; ///< Path to the project's configuration files.
    std::string logsPath; ///< Path to the project's log files.
    std::string buildPath; ///< Path to the project's build directory.

    std::string editorStartWorld; ///< The default world loaded in the editor.
    std::string gameStartWorld; ///< The default world loaded in the game.

public:
    ProjectConfig() = default;

    /// Constructs a ProjectConfig from JSON data.
    /// </summary>
    /// <param name="data">String containing JSON-formatted project configuration data.</param>
    ProjectConfig(std::string data);

    std::string getOpenTime() const { return openTime; }

    std::string getProjectPath() const { return projectPath; }
    std::string getProjectName() const { return projectName; }

    std::string getResourcesPath() const { return resourcesPath; }
    std::string getConfigPath() const { return configPath; }
    std::string getLogsPath() const { return configPath; }
    std::string getBuildPath() const { return buildPath; }

    std::string getEditorStartWorld() const { return editorStartWorld; }
    std::string getGameStartWorld() const { return gameStartWorld; }

    void setEditorStartWorld(const std::string& path) { editorStartWorld = path; }
};

/// <summary>
/// Singleton module responsible for managing project settings, environment, and configuration.
/// </summary>
class ProjectModule
{
    ProjectConfig m_projectConfig; ///< Stores the project configuration.

public:
    /// <summary>
    /// Retrieves the singleton instance of the ProjectModule.
    /// </summary>
    /// <returns>Reference to the ProjectModule instance.</returns>
    static ProjectModule& getInstance()
    {
        static ProjectModule ProjectModule;
        return ProjectModule;
    }

    /// <summary>
    /// Initializes the project module and sets up the project environment.
    /// </summary>
    void startup()
    {
        LOG_INFO("ProjectModule: Startup");
        setupProjectEnvironment();
    }

    /// <summary>
    /// Retrieves the project configuration.
    /// </summary>
    /// <returns>Reference to the ProjectConfig instance.</returns>
    ProjectConfig& getProjectConfig() { return m_projectConfig; }

    /// <summary>
    /// Shuts down the project module and saves the project configuration.
    /// </summary>
    void shutDown()
    {
        LOG_INFO("ProjectModule: Shut down");
        saveProjectConfig();
    }

private:
    /// <summary>
    /// Sets up the project environment by launching the Project Browser and retrieving configuration data.
    /// </summary>
    void setupProjectEnvironment();

    /// <summary>
    /// Saves the current project configuration to a file.
    /// </summary>
    void saveProjectConfig();
};

inline ProjectModule& PM = ProjectModule::getInstance();
