#pragma once

#include <memory>

#include <flecs.h>
#define FLECS_SYSTEM

struct ecs_world_deleter
{
	void operator()(ecs_world_t* world) const
	{
		if (world) 
		{
			ecs_fini(world);
		}
	}
};

class ECSModule
{
	std::unique_ptr<ecs_world_t, ecs_world_deleter> m_world;
public:
	ECSModule() {}
	~ECSModule() {}
	static ECSModule& getInstance()
	{
		static ECSModule ECSModule;
		return ECSModule;
	}

	void startUp()
	{
		m_world.reset(ecs_init());
	}

	void shutDown()
	{
		m_world.reset();
	}

	void ecsTick(float deltaTime)
	{
	
	}
};