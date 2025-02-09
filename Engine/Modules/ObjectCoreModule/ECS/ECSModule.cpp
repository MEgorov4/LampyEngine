#include "ECSModule.h"	
#include <format>
#include "../../LoggerModule/Logger.h"
#include "../../ProjectModule/ProjectModule.h"
#include "ECSLuaScriptsSystem.h"

void ECSModule::startup()
{
	LOG_INFO("ECSModule: startup");
	m_world = flecs::world();

	m_world.entity("GameStart").add(flecs::Phase);
	m_world.entity("GameEnd").add(flecs::Phase);

	loadInitialWorldState();

	ECSluaScriptsSystem::getInstance().registerSystem(m_world);
}

void ECSModule::loadInitialWorldState()
{
	m_world.entity("Bob").set<Position>({ 10, 20, 30 });
	m_world.entity("Alice").set<Position>({ 10, 20, 30 });
	m_world.entity("Penis").set<Position>({ 10, 20, 30 });
	m_world.entity("Penis").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
	m_world.entity("Chlen").set<Position>({ 10, 20, 30 });
	m_world.entity("Chlen").set<Script>({ProjectModule::getInstance().getProjectConfig().getResourcesPath() + "/b/test.lua"});
}

void ECSModule::clearWorld()
{
	m_world.each([](flecs::entity& e) {e.destruct();});
}

void ECSModule::startSystems()
{
	LOG_INFO("ECSModule: Start systems");

	ECSluaScriptsSystem::getInstance().startSystem(m_world);
	m_tickEnabled = true;
}

void ECSModule::stopSystems()
{
	LOG_INFO("ECSModule: Stop systems");
	m_tickEnabled = false;

	ECSluaScriptsSystem::getInstance().stopSystem(m_world);
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
		m_world.progress(deltaTime);
	}
}
