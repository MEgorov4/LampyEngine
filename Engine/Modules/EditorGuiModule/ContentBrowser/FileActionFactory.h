#pragma once 
#include <imgui.h>
#include <memory>
#include <vector>
#include <string>
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include <functional>
#include "../../FilesystemModule/FilesystemModule.h"

class IFileAction
{
public:
	virtual void execute(const std::string& filePath) = 0;
	virtual std::string getName() const = 0;
	virtual ~IFileAction() {}
};

class DeleteFileAction : public IFileAction
{
public:
	void execute(const std::string& filePath) override {
		FilesystemModule::getInstance().deleteFile(filePath);
	}
	std::string getName() const override { return "Delete"; }
};

class CopyPathAction : public IFileAction
{
public:
	void execute(const std::string& filePath) override 
	{
		FilesystemModule::getInstance().copyRelativePath(filePath);
	}

	std::string getName() const override { return "Copy path"; }
};

class CopyAbsolutePathAction : public IFileAction
{
public:
	void execute(const std::string& filePath) override 
	{
		FilesystemModule::getInstance().copyAbsolutePath(filePath);
	}
	std::string getName() const override { return "Copy absolute path"; }
};

class RenameFileAction : public IFileAction
{
public:
	void execute(const std::string& filePath) override
	{

	}
	std::string getName() const override { return "Rename"; }
};

class DuplicateFileAction : public IFileAction {
public:
	void execute(const std::string& filePath) override {
		FilesystemModule::getInstance().duplicateFileInDirectory(filePath);
	}

	std::string getName() const override { return "Duplicate"; }
};

class OpenWorldFileAction : public IFileAction {
public:
	void execute(const std::string& filePath) override {
		ECSModule::getInstance().loadWorldFromFile(filePath);
	}

	std::string getName() const override { return "Open world"; }
};

class SetWorldFileAsEditorDefaultAction : public IFileAction
{
public:
	void execute(const std::string& filePath) override
	{
		PM.getProjectConfig().setEditorStartWorld(filePath);
	}
	std::string getName() const override { return "Set as editor default world"; }
};

class SetWorldFileAsGameDefaultAction : public IFileAction
{
public:
	void execute(const std::string& filePath) override
	{

	}
	std::string getName() const override { return "Set as game default world"; }
};

class IFileActionFactory {
public:
	virtual std::vector<std::unique_ptr<IFileAction>> createActions() = 0;
	virtual ~IFileActionFactory() {}
};

class DefaultFileActionFactory : public IFileActionFactory {
public:
	std::vector<std::unique_ptr<IFileAction>> createActions() override {

		std::vector<std::unique_ptr<IFileAction>> actions;

		actions.push_back(std::make_unique<DeleteFileAction>());
		actions.push_back(std::make_unique<DuplicateFileAction>());
		actions.push_back(std::make_unique<CopyPathAction>());
		actions.push_back(std::make_unique<CopyAbsolutePathAction>());
		return actions;
	}
};

class WorldFileActionFactory : public IFileActionFactory {
public:
	std::vector<std::unique_ptr<IFileAction>> createActions() override {
		std::vector<std::unique_ptr<IFileAction>> actions;
		actions.push_back(std::make_unique<DeleteFileAction>());
		actions.push_back(std::make_unique<DuplicateFileAction>());
		actions.push_back(std::make_unique<CopyPathAction>());
		actions.push_back(std::make_unique<CopyAbsolutePathAction>());

		actions.push_back(std::make_unique<OpenWorldFileAction>());
		actions.push_back(std::make_unique<SetWorldFileAsEditorDefaultAction>());
		actions.push_back(std::make_unique<SetWorldFileAsGameDefaultAction>());
		return actions;
	}
};


class FileActionFactoryRegistry
{
public:
	static FileActionFactoryRegistry& getInstance() {
		static FileActionFactoryRegistry instance;
		return instance;
	}

	void registerFactory(const std::string& fileExtenstion, std::function<std::unique_ptr<IFileActionFactory>()> creator)
	{
		factories[fileExtenstion] = creator;
	}

	std::unique_ptr<IFileActionFactory> getFactory(const std::string& filePath)
	{
		std::string extension = getFileExtension(filePath);
		if (factories.find(extension) != factories.end())
		{
			return factories[extension]();
		}
		return std::make_unique<DefaultFileActionFactory>();
	}
 
private:
	std::unordered_map<std::string, std::function<std::unique_ptr<IFileActionFactory>()>> factories;

	std::string getFileExtension(const std::string& filePath)
	{
		size_t pos = filePath.find_last_of('.');
		return (pos != std::string::npos) ? filePath.substr(pos) : "";
	}
};
