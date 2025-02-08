#pragma once 

/// <summary>
/// Interface for defining different engine contexts (e.g., Game Mode, Editor Mode).
/// Each context must implement initialization, ticking, and shutdown behavior.
/// </summary>
class IEngineContext
{
public:
    IEngineContext() {}
    virtual ~IEngineContext() {}

    /// <summary>
    /// Initializes the engine context.
    /// This method should set up any necessary resources and configurations.
    /// </summary>
    virtual void initMinor() = 0;

    virtual void initMajor() = 0;
    /// <summary>
    /// Called every frame to update the engine context.
    /// </summary>
    /// <param name="deltaTime">Time elapsed since the last frame.</param>
    virtual void tick(float deltaTime) = 0;

    /// <summary>
    /// Shuts down the engine context and releases resources.
    /// </summary>
    virtual void shutDown() = 0;
};
