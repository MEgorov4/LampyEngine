#include "ECSModule.h"	
#include <format>
#include "../../LoggerModule/Logger.h"

void ECSModule::startUp()
{
	m_world = flecs::world();

	m_world.entity("Bob").set<Position>({10, 20, 30});
	m_world.entity("Alice").set<Position>({10, 20, 30});
	m_world.entity("Penis").set<Position>({10, 20, 30});
	m_world.entity("Chlen").set<Position>({10, 20, 30});

	LOG_INFO(std::format("Entity bob find state: {}", m_world.lookup("Bob").is_valid()));
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
	
}
