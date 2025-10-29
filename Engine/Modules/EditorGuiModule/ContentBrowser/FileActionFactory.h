#pragma once

#include <EngineMinimal.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <imgui.h>

namespace ECSModule
{
class ECSModule;
};
class IFileAction
{
  protected:
    ProjectModule::ProjectModule* m_projectModule;
    ECSModule::ECSModule* m_ecsModule;

  public:
    IFileAction() : m_projectModule(GCXM(ProjectModule::ProjectModule)), m_ecsModule(GCM(ECSModule::ECSModule))
    {
    }

    virtual void execute(const std::string& filePath) = 0;
    virtual std::string getName() const               = 0;

    virtual ~IFileAction()
    {
    }
};

class DeleteFileAction : public IFileAction
{
  public:
    DeleteFileAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
        Fs::deleteFile(filePath);
    }

    std::string getName() const override
    {
        return "Delete";
    }
};

class CopyPathAction : public IFileAction
{
  public:
    CopyPathAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
        Fs::copyRelativePathToClipboard(filePath, Fs::currentPath()); // TODO: ������ ��������� ������
    }

    std::string getName() const override
    {
        return "Copy path";
    }
};

class CopyAbsolutePathAction : public IFileAction
{
  public:
    CopyAbsolutePathAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
        Fs::copyAbsolutePathToClipboard(filePath);
    }

    std::string getName() const override
    {
        return "Copy absolute path";
    }
};

class RenameFileAction : public IFileAction
{
  public:
    RenameFileAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
    }

    std::string getName() const override
    {
        return "Rename";
    }
};

class DuplicateFileAction : public IFileAction
{
  public:
    DuplicateFileAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
        Fs::duplicateFileInDirectory(filePath);
    }

    std::string getName() const override
    {
        return "Duplicate";
    }
};

class OpenWorldFileAction : public IFileAction
{
  public:
    OpenWorldFileAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
        std::string worldData = Fs::readTextFile(filePath);
        m_ecsModule->openWorld(worldData);
    }

    std::string getName() const override
    {
        return "Open world";
    }
};

class SetWorldFileAsEditorDefaultAction : public IFileAction
{
  public:
    SetWorldFileAsEditorDefaultAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
        m_projectModule->getProjectConfig().setEditorStartWorld(filePath);
    }

    std::string getName() const override
    {
        return "Set as editor default world";
    }
};

class SetWorldFileAsGameDefaultAction : public IFileAction
{
  public:
    SetWorldFileAsGameDefaultAction() : IFileAction()
    {
    }

    void execute(const std::string& filePath) override
    {
    }

    std::string getName() const override
    {
        return "Set as game default world";
    }
};

class IFileActionFactory
{
  protected:

  public:
    IFileActionFactory()
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
    DefaultFileActionFactory() : IFileActionFactory()
    {
    }

    std::vector<std::unique_ptr<IFileAction>> createActions() override
    {
        std::vector<std::unique_ptr<IFileAction>> actions;

        actions.push_back(std::make_unique<DeleteFileAction>());
        actions.push_back(std::make_unique<DuplicateFileAction>());
        actions.push_back(std::make_unique<CopyPathAction>());
        actions.push_back(std::make_unique<CopyAbsolutePathAction>());
        return actions;
    }
};

class WorldFileActionFactory : public IFileActionFactory
{
  public:
    WorldFileActionFactory() : IFileActionFactory()
    {
    }

    std::vector<std::unique_ptr<IFileAction>> createActions() override
    {
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
    static FileActionFactoryRegistry& getInstance()
    {
        static FileActionFactoryRegistry instance;
        return instance;
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
