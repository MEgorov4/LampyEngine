#include "ECSModule.h"	
#include <format>
#include "../../LoggerModule/Logger.h"
#include "../../ProjectModule/ProjectModule.h"
#include "ECSLuaScriptsSystem.h"

void ECSModule::startUp()
{
	LOG_INFO("ECSModule: startUp");
	m_world = flecs::world();

	loadInitialWorldState();
	LOG_INFO(std::format("Entity bob find state: {}", m_world.lookup("Bob").is_valid()));

	ECSluaScriptsSystem::getInstance().registerSystem(m_world);
	auto move_sys = m_world.system<Position>()
		.each([](flecs::iter& it, size_t, Position& p) {
		p.x += 200 * it.delta_time();
			});
	move_sys.add(flecs::OnUpdate);

}

void ECSModule::loadInitialWorldState()
{
	m_world.entity("Bob").set<Position>({ 10, 20, 30 });
	m_world.entity("Alice").set<Position>({ 10, 20, 30 });
	m_world.entity("Penis").set<Position>({ 10, 20, 30 });
	m_world.entity("Chlen").set<Position>({ 10, 20, 30 });
	m_world.entity("Chlen").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/script.lua"});
}

void ECSModule::clearWorld()
{
	m_world.each([](flecs::entity& e) {e.destruct();});
}

void ECSModule::startSystems()
{
	LOG_INFO("ECSModule: start systems");
	m_tickEnabled = true;
}

void ECSModule::stopSystems()
{
	LOG_INFO("ECSMOdule: stop systems");
	m_tickEnabled = false;
	clearWorld();
	loadInitialWorldState();
}

flecs::world& ECSModule::getCurrentWorld() 
{ 
	return m_world; 
}

void ECSModule::shutDown()
{
	m_world.reset();
}

void ECSModule::ecsTick(float deltaTime)
{
	if (m_tickEnabled)
	{
		m_world.run_pipeline(0, deltaTime);
	}
}
