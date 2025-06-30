#pragma once

#include <memory>
#include <string>
#include <typeindex>

#include "ModuleRegistry.h"
#include "IModule.h"

class ModuleManager
{
public:
	template<typename T>
	std::shared_ptr<T> createModule(const std::string& name)
	{
		/*
		static_assert(std::is_base_of<IModule, T>::value, "T must inherit from IModule");
		*/

		std::shared_ptr<T> module = std::make_shared<T>();
		std::shared_ptr<IModule> basePtr = module;

		m_registry.registerModule(name, basePtr);
		m_modules.push_back(basePtr);

		return module;
	}
	void startupAll();
	void shutdownAll();

private:
	ModuleRegistry m_registry;

	std::vector<std::shared_ptr<IModule>> m_modules;
};

