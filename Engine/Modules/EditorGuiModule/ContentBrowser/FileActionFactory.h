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
protected:
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
    std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;

public:
    IFileAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                const std::shared_ptr<ECSModule::ECSModule>& ecsModule) : m_projectModule(projectModule),
                                                                          m_filesystemModule(filesystemModule),
                                                                          m_ecsModule(ecsModule)
    {
    }

    virtual void execute(const std::string& filePath) = 0;
    virtual std::string getName() const = 0;

    virtual ~IFileAction()
    {
    }
};

class DeleteFileAction : public IFileAction
{
public:
    DeleteFileAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                     const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                     const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
        m_filesystemModule->deleteFile(filePath);
    }

    std::string getName() const override { return "Delete"; }
};

class CopyPathAction : public IFileAction
{
public:
    CopyPathAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                   const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                   const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
        m_filesystemModule->copyRelativePath(filePath);
    }

    std::string getName() const override { return "Copy path"; }
};

class CopyAbsolutePathAction : public IFileAction
{
public:
    CopyAbsolutePathAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                           const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                           const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
        m_filesystemModule->copyAbsolutePath(filePath);
    }

    std::string getName() const override { return "Copy absolute path"; }
};

class RenameFileAction : public IFileAction
{
public:
    RenameFileAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                     const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                     const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
    }

    std::string getName() const override { return "Rename"; }
};

class DuplicateFileAction : public IFileAction
{
public:
    DuplicateFileAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                        const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                        const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
        m_filesystemModule->duplicateFileInDirectory(filePath);
    }

    std::string getName() const override { return "Duplicate"; }
};

class OpenWorldFileAction : public IFileAction
{
public:
    OpenWorldFileAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                        const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                        const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
        m_ecsModule->loadWorldFromFile(filePath);
    }

    std::string getName() const override { return "Open world"; }
};

class SetWorldFileAsEditorDefaultAction : public IFileAction
{
public:
    SetWorldFileAsEditorDefaultAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                                      const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                                      const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
        m_projectModule->getProjectConfig().setEditorStartWorld(filePath);
    }

    std::string getName() const override { return "Set as editor default world"; }
};

class SetWorldFileAsGameDefaultAction : public IFileAction
{
public:
    SetWorldFileAsGameDefaultAction(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                                    const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                                    const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileAction(projectModule, filesystemModule, ecsModule)
    {
    }

    void execute(const std::string& filePath) override
    {
    }

    std::string getName() const override { return "Set as game default world"; }
};

class IFileActionFactory
{
protected:
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
    std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;

public:
    IFileActionFactory(const std::shared_ptr<ProjectModule::ProjectModule> projectModule,
                       const std::shared_ptr<FilesystemModule::FilesystemModule> filesystemModule,
                       const std::shared_ptr<ECSModule::ECSModule> ecsModule) : m_projectModule(projectModule),
        m_filesystemModule(filesystemModule), m_ecsModule(ecsModule)
    {
    }

    virtual std::vector<std::unique_ptr<IFileAction>> createActions() = 0;

    virtual ~IFileActionFactory()
    {
    }
};

class DefaultFileActionFactory : public IFileActionFactory
{
public:
    DefaultFileActionFactory(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
        const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
        const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileActionFactory(projectModule, filesystemModule, ecsModule)
    {
    }

    std::vector<std::unique_ptr<IFileAction>> createActions() override
    {
        std::vector<std::unique_ptr<IFileAction>> actions;

        actions.push_back(std::make_unique<DeleteFileAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<DuplicateFileAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<CopyPathAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<CopyAbsolutePathAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        return actions;
    }
};

class WorldFileActionFactory : public IFileActionFactory
{
public:
    WorldFileActionFactory(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
        const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
        const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
        : IFileActionFactory(projectModule, filesystemModule, ecsModule)
    {
    }

    std::vector<std::unique_ptr<IFileAction>> createActions() override
    {
        std::vector<std::unique_ptr<IFileAction>> actions;
        actions.push_back(std::make_unique<DeleteFileAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<DuplicateFileAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<CopyPathAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<CopyAbsolutePathAction>(m_projectModule, m_filesystemModule, m_ecsModule));

        actions.push_back(std::make_unique<OpenWorldFileAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<SetWorldFileAsEditorDefaultAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        actions.push_back(std::make_unique<SetWorldFileAsGameDefaultAction>(m_projectModule, m_filesystemModule, m_ecsModule));
        return actions;
    }
};


class FileActionFactoryRegistry
{
    std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
    std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
    std::shared_ptr<ECSModule::ECSModule> m_ecsModule;

public:
    static FileActionFactoryRegistry& getInstance()
    {
        static FileActionFactoryRegistry instance;
        return instance;
    }

    void injectModules(const std::shared_ptr<ProjectModule::ProjectModule>& projectModule,
                       const std::shared_ptr<FilesystemModule::FilesystemModule>& filesystemModule,
                       const std::shared_ptr<ECSModule::ECSModule>& ecsModule)
    {
        m_projectModule = projectModule;
        m_filesystemModule = filesystemModule;
        m_ecsModule = ecsModule;
    }

    void registerFactory(const std::string& fileExtenstion,
                         std::function<std::unique_ptr<IFileActionFactory>()> creator)
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
        return std::make_unique<DefaultFileActionFactory>(m_projectModule, m_filesystemModule, m_ecsModule);
    }

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<IFileActionFactory>()>> factories;

    std::string getFileExtension(const std::string& filePath)
    {
        size_t pos = filePath.find_last_of('.');
        return (pos != std::string::npos) ? filePath.substr(pos) : "";
    }
};
