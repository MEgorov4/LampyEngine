#pragma once
#include <string>
#include <memory>
#include "../EngineContext/EngineContext.h"

namespace ImGUIModule
{
    class ImGUIModule;
}

class EditorGUIModule;

/// <summary>
/// Represents the Editor context, handling initialization, ticking, and shutdown of editor-specific modules.
/// </summary>
class Editor : public IEngineContext
{
    std::shared_ptr<EditorGUIModule> m_editorGUIModule;
    std::shared_ptr<ImGUIModule::ImGUIModule> m_imGUIModule;

public:
    ~Editor() override = default;

    /// <summary>
    /// Initializes the Editor context and starts necessary modules.
    /// </summary>
    void initMinor(ModuleManager* moduleManager) override;

    void initMajor(ModuleManager* moduleManager) override;
    /// <summary>
    /// Updates the Editor context every frame.
    /// </summary>
    /// <param name="deltaTime">Time elapsed since the last frame.</param>
    void tick(float deltaTime) override;

    /// <summary>
    /// Shuts down the Editor context and its associated modules.
    /// </summary>
    void shutdown() override;
};
