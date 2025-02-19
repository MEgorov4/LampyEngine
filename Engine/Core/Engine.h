#pragma once
#include "../EngineContext/EngineContext.h"
#include <memory>
/// <summary>
/// Manages the initialization, execution, and shutdown of the game engine.
/// </summary>
class Engine {
  std::unique_ptr<IEngineContext>
      m_engineContext; ///< Unique pointer to the engine context.

public:
  Engine();
  Engine(const Engine &app) = delete;
  ~Engine();
  const Engine &operator=(const Engine rhs) = delete;

  /// <summary>
  /// Runs the engine, handling startup, execution, and shutdown processes.
  /// </summary>
  void run();

private:
  /// <summary>
  /// Initializes all core engine modules (Window, Input, Rendering, Audio,
  /// ECS).
  /// </summary>
  void startupModules();

  /// <summary>
  /// Initializes the engine context object (e.g., Editor or Game mode).
  /// </summary>
  void startupEngineContextObject();
  void initMajorEngineContext();

  /// <summary>
  /// Runs the main game loop, handling delta time updates, rendering, and ECS
  /// updates.
  /// </summary>
  void engineTick();

  /// <summary>
  /// Shuts down the engine context object before closing the engine.
  /// </summary>
  void shutDownEngineContextObject();

  /// <summary>
  /// Shuts down all engine modules in a proper order.
  /// </summary>
  void shutDownModules();

  // Penis a potom chlen
  int idOnLoadInitialWorldState;
};
