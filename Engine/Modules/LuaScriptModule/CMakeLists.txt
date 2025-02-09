file(GLOB_RECURSE LUA_SCRIPT_MODULE_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.lua
)

format_source_group(${LUA_SCRIPT_MODULE_SOURCE})
add_library(LuaScriptModule ${LUA_SCRIPT_MODULE_SOURCE})

find_package(sol2 REQUIRED)

target_link_libraries(LuaScriptModule PUBLIC sol2::sol2  PRIVATE LoggerModule EventModule InputModule ResourceModule ProjectModule WindowModule)
