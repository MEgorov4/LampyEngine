#include "ModuleManager.h"


void ModuleManager::startupAll()
{
	for (auto& module : m_modules)
	{
		module->startup(m_registry);
	}
}

void ModuleManager::shutdownAll()
{
	for (auto& module : m_modules)
	{
		module->shutdown();
	}
}
