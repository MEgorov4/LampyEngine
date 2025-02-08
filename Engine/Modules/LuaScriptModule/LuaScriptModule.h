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

  void startUp() 
  {
      m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::string);
  }
    
  sol::state& getLuaState() 
  { 
      return m_luaState; 
  };
};
