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
          PhysicsModule
          TimeModule)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/Engine/Resources")

option(LAMPY_ENABLE_TRACY "Enable Tracy profiler" ON)

if(LAMPY_ENABLE_TRACY)
    find_package(tracy CONFIG REQUIRED) 
    target_link_libraries(Engine PRIVATE Tracy::TracyClient)
    target_compile_definitions(Engine PRIVATE TRACY_ENABLE)
endif()


set(RESOURCES_SRC_DIR "${CMAKE_SOURCE_DIR}/Resources")
set(RESOURCES_DST_DIR "${CMAKE_BINARY_DIR}/Engine/Resources")

if(EXISTS "${RESOURCES_SRC_DIR}")
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${RESOURCES_SRC_DIR}"
            "${RESOURCES_DST_DIR}"
    COMMENT "Sync Resources folder..."
)
else()
    message(WARNING "Resources directory not found at: ${RESOURCES_SRC_DIR}")
endif()