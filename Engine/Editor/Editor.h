#pragma once
#include <EngineMinimal.h>

namespace ImGUIModule
{
class ImGUIModule;
}

class EditorGUIModule;

class Editor : public IEngineContext
{
    std::shared_ptr<EditorGUIModule> m_editorGUIModule;
    std::shared_ptr<ImGUIModule::ImGUIModule> m_imGUIModule;

  public:
    ~Editor() override = default;

    void startupMinor(ContextLocator& locator) override;

    void startupMajor(ContextLocator& locator) override;

    void tick(float deltaTime) override;

    void shutdown() override;
};
