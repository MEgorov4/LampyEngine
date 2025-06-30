#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class IModule;
class ModuleRegistry
{
	ModuleRegistry(const ModuleRegistry&) = delete;
	ModuleRegistry(ModuleRegistry&&) = delete;
	ModuleRegistry& operator=(const ModuleRegistry&) = delete;
	ModuleRegistry& operator=(ModuleRegistry&&) = delete;
public:
	ModuleRegistry() = default;

	std::shared_ptr<IModule> getModule(const std::string& name) const;

	void registerModule(const std::string& name, std::shared_ptr<IModule> module);

private:
	std::unordered_map<std::string, std::shared_ptr<IModule>> m_modules;
};
