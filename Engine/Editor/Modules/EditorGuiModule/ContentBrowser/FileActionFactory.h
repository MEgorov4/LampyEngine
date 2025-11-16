#pragma once

#include "../Events.h"
#include <EngineMinimal.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ProjectModule/ProjectModule.h>
#include <imgui.h>
#include <filesystem>

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
    virtual bool requiresArgument() const
    {
        return false;
    }
    virtual void setArgument(const std::string& argument)
    {
        (void)argument;
    }
    virtual std::string argumentLabel() const
    {
        return "";
    }

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
        Events::EditorUI::FileDeleteRequest evt{};
        evt.filePath = filePath;
        GCEB().emit(evt);
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
        const std::string projectRoot = m_projectModule->getProjectConfig().getProjectPath();
        Fs::copyRelativePathToClipboard(filePath, projectRoot);
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
    std::string m_newName;

  public:
    RenameFileAction() : IFileAction()
    {
    }

    bool requiresArgument() const override
    {
        return true;
    }

    void setArgument(const std::string& argument) override
    {
        m_newName = argument;
    }

    std::string argumentLabel() const override
    {
        return "New name";
    }

    void execute(const std::string& filePath) override
    {
        if (m_newName.empty())
        {
            LT_LOG(LogVerbosity::Warning, "ContentBrowser", "Rename cancelled: empty name");
            return;
        }

        std::filesystem::path oldPath(filePath);
        std::filesystem::path newPath = oldPath.parent_path() / m_newName;

        if (Fs::exists(newPath.string()))
        {
            LT_LOG(LogVerbosity::Error, "ContentBrowser", "Rename failed, target exists: " + newPath.string());
            Events::EditorUI::FileOperationCompleted evt{"rename", filePath, false};
            GCEB().emit(evt);
            return;
        }

        std::error_code ec;
        std::filesystem::rename(oldPath, newPath, ec);
        if (ec)
        {
            LT_LOG(LogVerbosity::Error, "ContentBrowser",
                   "Rename failed for '" + filePath + "': " + ec.message());
            Events::EditorUI::FileOperationCompleted evt{"rename", filePath, false};
            GCEB().emit(evt);
            return;
        }

        LT_LOG(LogVerbosity::Info, "ContentBrowser",
               "Renamed file: " + oldPath.filename().string() + " -> " + newPath.filename().string());

        Events::EditorUI::FileOperationCompleted evt{"rename", newPath.string(), true};
        GCEB().emit(evt);
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
        Events::EditorUI::FileDuplicateRequest evt{};
        evt.filePath = filePath;
        GCEB().emit(evt);
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
        Events::EditorUI::WorldLoadRequest evt{};
        evt.filePath = filePath;
        GCEB().emit(evt);
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
        Events::EditorUI::ProjectStartWorldSet evt{};
        evt.worldPath = filePath;
        GCEB().emit(evt);

        m_projectModule->getProjectConfig().setEditorStartWorld(filePath);
        m_projectModule->saveProjectConfigNow();

        Events::EditorUI::WorldDefaultChanged changed{};
        changed.worldPath = filePath;
        changed.target    = Events::EditorUI::WorldDefaultTarget::Editor;
        GCEB().emit(changed);
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
        m_projectModule->getProjectConfig().setGameStartWorld(filePath);
        m_projectModule->saveProjectConfigNow();

        Events::EditorUI::WorldDefaultChanged changed{};
        changed.worldPath = filePath;
        changed.target    = Events::EditorUI::WorldDefaultTarget::Game;
        GCEB().emit(changed);
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
        actions.push_back(std::make_unique<RenameFileAction>());
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
        actions.push_back(std::make_unique<RenameFileAction>());

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
