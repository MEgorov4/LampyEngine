#pragma once
#include <EngineMinimal.h>
#include <sol/forward.hpp>
#include <sol/reference.hpp>
#include <sol/sol.hpp>
#include "Foundation/Memory/ResourceAllocator.h"

using EngineCore::Foundation::ResourceAllocator;

class LuaScriptExecutor
{
    std::vector<sol::table, ResourceAllocator<sol::table>> m_tables;

  public:
    void addScript(const sol::table& table);
    void removeScript(const sol::table& table);
    void executeStart();
    void executeUpdate(float deltaSeconds);
};
