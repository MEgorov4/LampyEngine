file(GLOB_RECURSE ENGINE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

format_source_group(${ENGINE_SOURCE})
add_executable(Engine ${ENGINE_SOURCE})

target_link_libraries(
  Engine
  PRIVATE InputModule
          AudioModule
          WindowModule
          RenderModule
          ResourceModule
          EditorModule
          EngineContext
          ObjectCoreModule
          LuaScriptModule
          ShaderCompilerModule
          PhysicsModule)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/Engine/Resources")

option(LAMPY_ENABLE_TRACY "Enable Tracy profiler" ON)

if(LAMPY_ENABLE_TRACY)
    find_package(tracy CONFIG REQUIRED) 
    target_link_libraries(Engine PRIVATE Tracy::TracyClient)
    target_compile_definitions(Engine PRIVATE TRACY_ENABLE)
endif()
