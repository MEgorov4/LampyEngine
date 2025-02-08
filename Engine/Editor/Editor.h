#pragma once
#include <string>
#include "../EngineContext/EngineContext.h"

/// <summary>
/// Represents the Editor context, handling initialization, ticking, and shutdown of editor-specific modules.
/// </summary>
class Editor : public IEngineContext
{
public:
    Editor() {}
    virtual ~Editor() {}

    /// <summary>
    /// Initializes the Editor context and starts necessary modules.
    /// </summary>
    void initMinor() override;
    
    void initMajor() override;
    /// <summary>
    /// Starts up editor-specific modules (e.g., GUI, Project Management).
    /// </summary>
    void startupEditorModules();

    /// <summary>
    /// Updates the Editor context every frame.
    /// </summary>
    /// <param name="deltaTime">Time elapsed since the last frame.</param>
    void tick(float deltaTime) override;

    /// <summary>
    /// Shuts down the Editor context and its associated modules.
    /// </summary>
    void shutDown() override;

    /// <summary>
    /// Shuts down editor-specific modules.
    /// </summary>
    void shutDownEditorModules();
};
