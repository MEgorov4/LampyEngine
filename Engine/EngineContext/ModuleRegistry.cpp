#include "ModuleRegistry.h"
#include "IModule.h"

std::shared_ptr<IModule> ModuleRegistry::getModule(const std::string& name) const 
{
	return m_modules.at(name);
}

void ModuleRegistry::registerModule(const std::string& name, std::shared_ptr<IModule> module)
{
	m_modules[name] = std::shared_ptr<IModule>(module);
}

