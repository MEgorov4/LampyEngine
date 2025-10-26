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
          EventModule
          ObjectCoreModule
          LuaScriptModule
          ShaderCompilerModule
          PhysicsModule)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/Engine/Resources")

add_custom_command(
  TARGET Engine
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCE_DIR}" "${OUTPUT_DIR}"
  COMMENT "Copying Resources folder to output directory")
