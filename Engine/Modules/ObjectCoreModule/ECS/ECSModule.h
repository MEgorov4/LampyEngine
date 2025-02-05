#pragma once
#include <memory>
#include <flecs.h>

struct Position
{
	float x;
	float y;
	float z;
};

class ECSModule
{
	flecs::world m_world;
public:
	static ECSModule& getInstance()
	{
		static ECSModule ECSModule;
		return ECSModule;
	}

	void startUp();

	flecs::world& getCurrentWorld();

	void ecsTick(float deltaTime);

	void shutDown();
};