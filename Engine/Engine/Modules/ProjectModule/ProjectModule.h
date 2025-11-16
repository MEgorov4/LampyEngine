#pragma once 

#include <EngineMinimal.h>
#include <optional>

namespace ProjectModule
{
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

		std::string getOpenTime() const;

		std::string getProjectPath() const;
		std::string getProjectName() const;

		std::string getResourcesPath() const;
		std::string getConfigPath() const;
		std::string getLogsPath() const;
		std::string getBuildPath() const;

		std::string getEditorStartWorld() const;
		std::string getGameStartWorld() const;

		void setEditorStartWorld(const std::string& path);
		void setGameStartWorld(const std::string& path);
	};

	/// <summary>
	/// Singleton module responsible for managing project settings, environment, and configuration.
	/// </summary>
	class ProjectModule : public IModule
	{
		ProjectConfig m_projectConfig; ///< Stores the project configuration.
		std::optional<std::string> m_projectFileOverride;
	public:
		/// <summary>
		/// Initializes the project module and sets up the project environment.
		/// </summary>
		void startup() override;

		/// <summary>
		/// Retrieves the project configuration.
		/// </summary>
		/// <returns>Reference to the ProjectConfig instance.</returns>
		ProjectConfig& getProjectConfig();

		/// <summary>
		/// Shuts down the project module and saves the project configuration.
		/// </summary>
		void shutdown() override;

		void setProjectFileOverride(const std::string& projectFile);
		void saveProjectConfigNow();

	private:
		/// <summary>
		/// Sets up the project environment by launching the Project Browser and retrieving configuration data.
		/// </summary>
		void setupProjectEnvironment();

		/// <summary>
		/// Saves the current project configuration to a file.
		/// </summary>
		void saveProjectConfig();

		bool loadProjectFromFile(const std::string& filePath);
	};
}
