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
    
  void processCommand(const std::string& command);
  sol::state& getLuaState() 
  { 
      return m_luaState; 
  };

private:
  void registerLogger();
  void registerEvent();
  void registerInputModuleEvents();
  void registerAudioModule();
  void registerMathTypes();
  void registerECSModule();
};
