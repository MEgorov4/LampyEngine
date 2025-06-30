#pragma once 

#include "ModuleRegistry.h"

class IModule
{
public:
	virtual void startup(const ModuleRegistry& registry) = 0;
	virtual void shutdown() = 0;
	virtual ~IModule() = default;
};