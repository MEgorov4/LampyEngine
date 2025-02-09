#pragma once

#include <sol/sol.hpp>

class LuaScriptModule {

  LuaScriptModule() = default;
  LuaScriptModule &operator=(const LuaScriptModule &) = delete;
  LuaScriptModule(const LuaScriptModule &) = delete;
  
  sol::state m_luaState;
public:
  static LuaScriptModule &getInstance() 
  {
    static LuaScriptModule luaScriptModule;
    return luaScriptModule;
  }

  void startup();
  void registerLogger();
  void registerEvent();
  void registerInputModuleEvents();
  sol::state& getLuaState() 
  { 
      return m_luaState; 
  };
};
