#pragma once 

class IEngineContext
{
public:
	IEngineContext() {};
	virtual ~IEngineContext() {};

	virtual void init() = 0;
	virtual void tick(float deltaTime) = 0;
	virtual void shutDown() = 0;
};