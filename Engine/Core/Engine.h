#pragma once
#include <memory>

#include "../EngineContext/EngineContext.h"
#include "../EngineContext/ModuleManager.h"

namespace ECSModule
{
	class ECSModule;
}

class PhysicsModule;

namespace RenderModule
{
	class RenderModule;
}

namespace WindowModule
{
	class WindowModule;
}

/// <summary>
/// Manages the initialization, execution, and shutdown of the game engine.
/// </summary>
class Engine {
	std::shared_ptr<WindowModule::WindowModule> m_windowModule;
	std::shared_ptr<RenderModule::RenderModule> m_renderModule;
	std::shared_ptr<PhysicsModule> m_physicsModule;
	std::shared_ptr<ECSModule::ECSModule> m_ecsModule;
		
	std::unique_ptr<IEngineContext>
		m_engineContext; ///< Unique pointer to the engine context.
	std::unique_ptr<ModuleManager>
		m_moduleManager;
public:
	Engine() = default;
	~Engine() = default;
	Engine(const Engine& app) = delete;
	Engine(Engine&& app) = delete;
	Engine& operator=(const Engine& rhs) = delete;
	Engine& operator=(Engine&& rhs) = delete;

	/// <summary>
	/// Runs the engine, handling startup, execution, and shutdown processes.
	/// </summary>
	void run();
	void ContextCreate();
	void ContextMajorInit() const;
	void ContextMinorInit() const;

private:
	/// <summary>
	/// Initializes all core engine modules (Window, Input, Rendering, Audio,
	/// ECS...).
	/// </summary>
	void startup();

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
};
