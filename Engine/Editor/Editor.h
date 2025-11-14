#pragma once
#include <EngineMinimal.h>

namespace ImGUIModule
{
class ImGUIModule;
}

class EditorGUIModule;

class EditorApplication : public Application
{
    std::shared_ptr<EditorGUIModule> m_editorGUIModule;
    std::shared_ptr<ImGUIModule::ImGUIModule> m_imGUIModule;

  public:
    ~EditorApplication() override = default;
    
    void startup();
    void onStartupMinor(ContextLocator* locator) override;
    void onStartupMajor(ContextLocator* locator) override;
    void onShutdown() override;

    void render() override;
    void tick(float deltaTime) override;

  protected:
    void configureModules(ModuleConfigRegistry& registry) override;
};
